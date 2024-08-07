#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>
#include "coroutine.h"
#include "server_connection.h"

namespace pine
{
  /// @brief A server that accepts connections from clients.
  class server
  {
    friend class server_connection;

  public:
    /// @brief Construct a server with the given asio context and port.
    explicit server(const char* port = "80");

    /// @brief Start listening for connections.
    void start(std::error_code& ec);

    /// @brief Stop listening for connections.
    void stop();

    /// @brief Disconnect a client.
    /// @param client_id Id of the client to disconnect.	
    /// @return An asynchronous task completed when the client has been
    /// disconnected.
    async_task disconnect_client(uint64_t const& client_id);

  private:
    /// @brief Accept clients.
    /// This function waits for clients to connect and creates a server
    /// connection for each client.
    /// @param ec An error code to be set if an error occurs.
    /// @return An asynchronous task completed when the server has stopped
    /// listening.
    void accept_clients(std::error_code& ec);

    std::mutex delete_clients_mutex;
    std::mutex mutate_clients_mutex;

    std::unordered_map<uint64_t, std::shared_ptr<server_connection>> clients;

    const char* port;
  #ifdef _WIN32
    SOCKET server_socket = INVALID_SOCKET;
    addrinfo* address_info = nullptr;
  #else
    int server_socket = -1;
  #endif

    bool is_listening = false;
    std::jthread acceptor_thread;
    std::jthread delete_clients_thread;

  public:
    /// @brief Call this function to add a callback that will be executed when 
    /// a new client attemps to connect to the server.
    /// @return A reference to this server.
    server& on_connection_attempt(std::function < async_task(
      server&,
      std::shared_ptr<server_connection> const&)> const& callback
    );

    /// @brief Call this function to add a callback that will be executed when 
    /// a client fails
    /// to connect to the server.
    /// @return A reference to this server.
    server& on_connection_failed(std::function < async_task(
      server&,
      std::shared_ptr<server_connection> const&)> const& callback
    );

    /// @brief Call this function to add a callback that will be executed when
    /// a client successfully
    /// connects to the server.
    /// @return A reference to this server.
    server& on_connection(std::function < async_task(
      server&,
      std::shared_ptr<server_connection> const&)> const& callback
    );

    /// @brief Call this function to add a callback that will be executed when
    /// the server is ready
    /// to accept connections.
    /// @return A reference to this server.
    server& on_ready(std::function < async_task(
      server&)> const& callback
    );

  private:
    std::vector<std::function<async_task(
      server&,
      std::shared_ptr<server_connection> const&)>
    > on_connection_attemps_callbacks;

    std::vector<std::function<async_task(
      server&,
      std::shared_ptr<server_connection> const&)>
    > on_connection_failed_callbacks;

    std::vector<std::function<async_task(
      server&,
      std::shared_ptr<server_connection> const&)>
    > on_connection_callbacks;

    std::vector < std::function < async_task(
      server&)>
    > on_ready_callbacks;
  };
}