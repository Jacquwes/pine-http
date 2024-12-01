#include <cstdint>
#include <filesystem>
#include <functional>
#include <initializer_list>
#include <loguru.hpp>
#include <memory>
#include <pine/error.h>
#include <pine/expected.h>
#include <pine/http.h>
#include <pine/http_request.h>
#include <pine/http_response.h>
#include <pine/iocp.h>
#include <pine/socket.h>
#include <pine/route_node.h>
#include <pine/route_path.h>
#include <pine/route_tree.h>
#include <pine/server.h>
#include <pine/server_connection.h>
#include <string>
#include <type_traits>
#include <vector>

namespace pine
{
  server::server(uint16_t port)
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
    auto socket_result = socket::create(port);
    if (!socket_result)
      return std::make_unexpected(socket_result.error());
    server_socket = std::move(socket_result.value());

    if (const auto& listen_result = server_socket.listen();
        !listen_result)
      return std::make_unexpected(listen_result.error());

    is_listening = true;

    LOG_F(INFO, "Server socket initialized. Will start receiving requests soon.");

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

    server_socket.close();

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

    LOG_F(INFO, "Posting 10 accept operations.");

    for (size_t i = 0; i < 10; i++)
    {
      if (!iocp_.post(iocp_operation::accept, server_socket, {}, 0))
        return std::make_unexpected(error(error_code::iocp_error,
                                          "Failed to post accept operation: " + std::to_string(WSAGetLastError())));
    }

    return {};
  }

  void server::remove_client(uint64_t const& client_id)
  {
    std::unique_lock lock{ clients_mutex_ };

    const auto& it = clients.find(client_id);
    if (it == clients.end())
    {
      LOG_F(WARNING, "Attempting to remove non-existent client: % zu", client_id);
    }

    LOG_F(INFO, "Removing client: %zu. Remaining clients: %llu", client_id, clients.size() - 1);

    auto&& client = std::move(it->second);
    clients.erase(it);
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
      {
        LOG_F(WARNING, "Client not found: %zu", data->socket);
        return;
      }
      client = it->second;
    }
    client->on_write_raw(data);
  }
}
