#pragma once

#include <connection.h>
#include <coroutine.h>
#include <http_request.h>
#include <http_response.h>
#include <memory>
#include <system_error>
#include <WinSock2.h>

namespace pine
{
  class server;

  /// @brief A connection to a client.
  class server_connection
    : public connection
    , public std::enable_shared_from_this<server_connection>
  {
  public:
    /// @brief Construct a server connection with the given socket and server.
    explicit server_connection(SOCKET socket, pine::server& server);

    async_operation<void> handle_request(const http_request& request) const;

    /// @brief Receive an HTTP request.
    /// @return An asynchronous task completed when the request has been received.
    async_operation<http_request> receive_request() const;

    /// @brief Send an HTTP response.
    /// @param response The response to send.
    /// @return An asynchronous task completed when the response has been sent.
    async_operation<void> send_response(http_response const& response) const;

    /// @brief Start listening for messages from the client.
    /// @return An asynchronous task completed when the connection has been closed.
    async_operation<void> start();

  private:
    /// @brief Whether the connection is connected.
    bool is_connected = true;

    /// @brief The server that the connection is connected to.
    server& server;
  };
}