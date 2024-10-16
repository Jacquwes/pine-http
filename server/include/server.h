#pragma once

#include <coroutine.h>
#include <cstdint>
#include <error.h>
#include <expected.h>
#include <functional>
#include <memory>
#include <mutex>
#include <system_error>
#include <thread>
#include <unordered_map>
#include <vector>
#include <WinSock2.h>

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
    std::expected<void, pine::error> start();

    /// @brief Stop listening for connections.
    void stop();

    /// @brief Disconnect a client.
    /// @param client_id Id of the client to disconnect.
    /// @return An asynchronous task completed when the client has been
    /// disconnected.
    async_operation<void> disconnect_client(
      uint64_t const& client_id);

  private:
    /// @brief Accept clients.
    /// This function waits for clients to connect and creates a server
    /// connection for each client.
    /// @return An asynchronous task completed when the server has stopped
    /// listening.
    std::expected<void, pine::error> accept_clients();

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
    /// a new client attempts to connect to the server.
    /// @return A reference to this server.
    server&
      on_connection_attempt(
      std::function<async_operation<void>(server&,
      std::shared_ptr<server_connection> const&)> const& callback
      );

    /// @brief Call this function to add a callback that will be executed when
    /// a client fails
    /// to connect to the server.
    /// @return A reference to this server.
    server&
      on_connection_failed(
      std::function < async_operation<void>(
      server&,
      std::shared_ptr<server_connection> const&)> const& callback
      );

    /// @brief Call this function to add a callback that will be executed when
    /// a client successfully
    /// connects to the server.
    /// @return A reference to this server.
    server&
      on_connection(std::function < async_operation<void>(
      server&,
      std::shared_ptr<server_connection> const&)> const& callback
      );

    /// @brief Call this function to add a callback that will be executed when
    /// the server is ready
    /// to accept connections.
    /// @return A reference to this server.
    server&
      on_ready(std::function < async_operation<void>(
      server&)> const& callback
    );

  private:
    std::vector<std::function<async_operation<void>(
      server&,
      std::shared_ptr<server_connection> const&)>
    > on_connection_attemps_callbacks;

    std::vector<std::function<async_operation<void>(
      server&,
      std::shared_ptr<server_connection> const&)>
    > on_connection_failed_callbacks;

    std::vector<std::function<async_operation<void>(
      server&,
      std::shared_ptr<server_connection> const&)>
    > on_connection_callbacks;

    std::vector < std::function < async_operation<void>(
      server&)>
    > on_ready_callbacks;
  };
}