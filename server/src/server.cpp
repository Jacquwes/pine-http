#include <coroutine.h>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <type_traits>
#include "server.h"
#include "server_connection.h"
#include "wsa.h"

namespace pine
{
  server::server(const char* port)
    : port(port)
  {}

  void server::start(std::error_code& ec)
  {
    initialize_wsa(ec);
    if (ec)
      return;

    this->address_info = get_address_info(nullptr, port, ec);
    if (ec)
      return;

    this->server_socket = create_socket(this->address_info, ec);
    if (ec)
      return;

    bind_socket(server_socket, this->address_info, ec);
    if (ec)
      return;

    listen_socket(server_socket, 1000, ec);
    if (ec)
      return;

    this->is_listening = true;

    this->accept_clients(ec);
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

  void server::accept_clients(std::error_code& ec)
  {
    for (const auto& callback : on_ready_callbacks)
    {
      callback(*this);
    }

    while (this->is_listening)
    {
      auto client_socket = accept_socket(this->server_socket,
                                         this->address_info,
                                         ec);
      if (ec)
      {
        for (const auto& callback : this->on_connection_failed_callbacks)
        {
          callback(*this, nullptr);
        }
        continue;
      }

      auto client = std::make_shared<server_connection>(client_socket);

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

      client->start();

      lock.unlock();
    }
  }

  async_task server::disconnect_client(uint64_t const& client_id)
  {
    std::unique_lock lock{ mutate_clients_mutex };

    auto client = clients.find(client_id);

    if (client == clients.end())
    {
      co_return;
    }

    client->second->close();

    clients.erase(client_id);

    co_return;
  }

  server& server::on_connection_attempt(
    std::function<
    async_task(server&, std::shared_ptr<server_connection>const&)
    > const& callback)
  {
    on_connection_attemps_callbacks.push_back(callback);
    return *this;
  }

  server& server::on_connection_failed(
    std::function<
    async_task(server&, std::shared_ptr<server_connection>const&)
    > const& callback)
  {
    on_connection_failed_callbacks.push_back(callback);
    return *this;
  }

  server& server::on_connection(
    std::function<
    async_task(server&, std::shared_ptr<server_connection>const&)
    > const& callback)
  {
    on_connection_callbacks.push_back(callback);
    return *this;
  }

  server& server::on_ready(std::function<async_task(server&)> const& callback)
  {
    on_ready_callbacks.push_back(callback);
    return *this;
  }
}
