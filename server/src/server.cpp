#include <coroutine.h>
#include <cstdint>
#include <error.h>
#include <expected.h>
#include <functional>
#include <memory>
#include <system_error>
#include <wsa.h>
#include "server.h"
#include "server_connection.h"

namespace pine
{
  server::server(const char* port)
    : port(port)
  {}

  std::expected<void, std::error_code> server::start()
  {
    if (const auto& init_result = initialize_wsa();
        !init_result)
      return std::make_unexpected(init_result.error());

    const auto& get_address_result = get_address_info(nullptr, port);
    if (!get_address_result)
      return std::make_unexpected(get_address_result.error());
    this->address_info = get_address_result.value();

    const auto& socket_result = create_socket(this->address_info);
    if (!socket_result)
      return std::make_unexpected(socket_result.error());
    this->server_socket = socket_result.value();

    if (const auto& bind_result = bind_socket(server_socket, this->address_info);
        !bind_result)
      return std::make_unexpected(bind_result.error());

    if (const auto& listen_result = listen_socket(server_socket, 1000);
        !listen_result)
      return std::make_unexpected(listen_result.error());

    this->is_listening = true;

    if (const auto& accept_result = this->accept_clients();
        !accept_result)
      return std::make_unexpected(accept_result.error());

    return {};
  }

  void server::stop()
  {
    is_listening = false;

    close_socket(this->server_socket);

    delete this->address_info;

    for (const auto& [id, client] : this->clients)
    {
      client->close();
    }

    this->clients.clear();
  }

  std::expected<void, std::error_code> server::accept_clients()
  {
    for (const auto& callback : on_ready_callbacks)
    {
      callback(*this);
    }

    while (this->is_listening)
    {
      const auto& accept_socket_result = accept_socket(this->server_socket,
                                                       this->address_info);
      if (!accept_socket_result)
      {
        for (const auto& callback : this->on_connection_failed_callbacks)
        {
          callback(*this, nullptr);
        }
        continue;
      }

      const auto& client_socket = accept_socket_result.value();
      const auto& client = std::make_shared<server_connection>(client_socket);

      for (const auto& callback : this->on_connection_attemps_callbacks)
      {
        callback(*this, client);
      }

      std::unique_lock lock{ mutate_clients_mutex };

      this->clients[client->get_id()] = client;

      for (const auto& callback : this->on_connection_callbacks)
      {
        callback(*this, client);
      }

      if (const auto& start_result = client->start().await_resume(); !start_result)
      {
        this->clients.erase(client->get_id());
      }

      lock.unlock();
    }

    return {};
  }

  async_operation<void, std::error_code> server::disconnect_client(uint64_t const& client_id)
  {
    std::unique_lock lock{ mutate_clients_mutex };

    const auto& client = clients.find(client_id);

    if (client == clients.end())
    {
      co_return make_error_code(error::client_not_found);
    }

    client->second->close();

    clients.erase(client_id);

    co_return make_error_code(error::success);
  }

  server& server::on_connection_attempt(
    std::function<
    async_operation<void, std::error_code>(server&, std::shared_ptr<server_connection>const&)
    > const& callback)
  {
    on_connection_attemps_callbacks.push_back(callback);
    return *this;
  }

  server& server::on_connection_failed(
    std::function<
    async_operation<void, std::error_code>(server&, std::shared_ptr<server_connection>const&)
    > const& callback)
  {
    on_connection_failed_callbacks.push_back(callback);
    return *this;
  }

  server& server::on_connection(
    std::function<
    async_operation<void, std::error_code>(server&, std::shared_ptr<server_connection>const&)
    > const& callback)
  {
    on_connection_callbacks.push_back(callback);
    return *this;
  }

  server& server::on_ready(std::function<async_operation<void, std::error_code>(server&)> const& callback)
  {
    on_ready_callbacks.push_back(callback);
    return *this;
  }
}
