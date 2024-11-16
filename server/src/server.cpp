#include <coroutine.h>
#include <cstdint>
#include <error.h>
#include <expected.h>
#include <filesystem>
#include <functional>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <initializer_list>
#include <iocp.h>
#include <loguru.hpp>
#include <memory>
#include <route_node.h>
#include <route_path.h>
#include <route_tree.h>
#include <server.h>
#include <server_connection.h>
#include <string>
#include <type_traits>
#include <vector>
#include <wsa.h>

namespace pine
{
  server::server(const char* port)
    : iocp_{},
    port{ port }
  {
    for (const auto& [status_code, status_string] : http_status_strings)
    {
      error_handlers[status_code] =
        [&status_string, &status_code](const auto&, auto& res)
        {
          res.set_body(status_string);
          res.set_status(status_code);
        };
    }
  }

  std::expected<void, error> server::start()
  {
    if (const auto& init_result = initialize_wsa();
        !init_result)
      return std::make_unexpected(init_result.error());

    const auto& get_address_result = get_address_info(nullptr, port);
    if (!get_address_result)
      return std::make_unexpected(get_address_result.error());
    address_info = get_address_result.value();

    const auto& socket_result = create_socket(address_info);
    if (!socket_result)
      return std::make_unexpected(socket_result.error());
    server_socket = socket_result.value();

    if (const auto& bind_result = bind_socket(server_socket,
                                              address_info);
        !bind_result)
      return std::make_unexpected(bind_result.error());

    if (const auto& listen_result = listen_socket(server_socket, 1000);
        !listen_result)
      return std::make_unexpected(listen_result.error());

    is_listening = true;

    LOG_F(INFO, "Server socket initialized. Will start receiving requests soon.");

    int opt = 1;
    // Allow the socket to be reused.
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
               (char*)&opt, sizeof(opt));

    // Disable Nagle's algorithm.
    setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY,
               (char*)&opt, sizeof(opt));

    // Increase the buffer size.
    int buffer_size = 64 * 1024;
    setsockopt(server_socket, SOL_SOCKET, SO_RCVBUF,
               (char*)&buffer_size, sizeof(buffer_size));
    setsockopt(server_socket, SOL_SOCKET, SO_SNDBUF,
               (char*)&buffer_size, sizeof(buffer_size));

    iocp_.init(server_socket);
    iocp_.set_on_accept([this](const pine::iocp_operation_data* data) { on_accept(data); });
    iocp_.set_on_read([this](const pine::iocp_operation_data* data) { on_read(data); });
    iocp_.set_on_write([this](const pine::iocp_operation_data* data) { on_write(data); });
    iocp_.associate(server_socket);

    LOG_F(INFO, "IOCP initialized.");

    if (const auto& accept_result = accept_clients();
        !accept_result)
      return std::make_unexpected(accept_result.error());

    LOG_F(INFO, "Accepting clients.");

    return {};
  }

  void server::stop()
  {
    is_listening = false;

    LOG_F(INFO, "Stopping server.");

    close_socket(server_socket);

    delete address_info;

    for (const auto& [id, client] : clients)
    {
      client->close();
    }

    clients.clear();

    LOG_F(INFO, "Server stopped.");
  }

  std::expected<void, error> server::accept_clients()
  {
    // Post 10 accept operations so there is no delay starting a new thread
    // when a client connects.

    LOG_F(INFO, "Posting 100 accept operations.");

    for (size_t i = 0; i < 100; i++)
    {
      if (!iocp_.post(iocp_operation::accept, server_socket, {}, 0))
        return std::make_unexpected(error(error_code::iocp_error,
                                          "Failed to post accept operation: " + std::to_string(WSAGetLastError())));
    }

    return {};
  }

  async_operation<void> server::remove_client(uint64_t const& client_id)
  {
    std::unique_lock lock{ clients_mutex_ };

    const auto& it = clients.find(client_id);
    if (it == clients.end())
    {
      LOG_F(WARNING, "Attempting to remove non-existent client: % zu", client_id);
      co_return error(error_code::client_not_found,
                      "The client was not found.");
    }

    LOG_F(INFO, "Removing client: %zu. Remaining clients: %llu", client_id, clients.size() - 1);

    auto&& client = std::move(it->second);
    clients.erase(it);

    co_return{};
  }

  route_node&
    server::add_route(route_path path,
                      const std::function<void(const http_request&,
                                               http_response&)>& handler,
                      const std::initializer_list<http_method>& methods)
  {
    auto& new_route = routes.add_route(path);
    for (const auto& method : methods)
    {
      new_route.add_handler(method,
                            std::make_unique<route_tree::handler_type>(handler));
    }

    LOG_F(INFO, "Added route: %s", path.get().data());

    return new_route;
  }

  route_node& server::add_static_route(route_path path,
                                       std::filesystem::path&&
                                       location)
  {
    auto& new_route = routes.add_route(path);

    new_route.serve_files(std::move(location));

    LOG_F(INFO, "Added static route: %s", path.get().data());

    return new_route;
  }

  void server::add_error_handler(http_status status, callback_function&& callback)
  {
    error_handlers[status] = std::move(callback);
  }

  const route_node&
    server::get_route(std::string_view path) const
  {
    return routes.find_route(path);
  }

  void server::on_accept(const iocp_operation_data* data)
  {
    const auto& client_socket = data->socket;
    LOG_F(INFO, "New client connection accepted: %zu", client_socket);

    const auto& client = std::make_shared<server_connection<buffer_size>>(client_socket,
                                                                          *this);
    {
      std::unique_lock lock{ clients_mutex_ };
      clients[data->socket] = client;
      LOG_F(INFO, "Client added to the list: %zu", client_socket);
    }

    client->post_read();

    iocp_.post(iocp_operation::accept, server_socket, {}, 0);
  }

  void server::on_read(const iocp_operation_data* data)
  {
    std::shared_ptr<server_connection<buffer_size>> client;
    {
      std::shared_lock lock{ clients_mutex_ };

      const auto& it = clients.find(data->socket);
      if (it == clients.end())
        return;
      client = it->second;
    }
    client->on_read_raw(data);
  }

  void server::on_write(const iocp_operation_data* data)
  {
    std::shared_ptr<server_connection<buffer_size>> client;
    {
      std::shared_lock lock{ clients_mutex_ };

      const auto& it = clients.find(data->socket);
      if (it == clients.end())
        return;
      client = it->second;
    }
    client->on_write_raw(data);
  }
}
