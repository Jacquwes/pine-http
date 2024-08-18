#pragma once

#include <memory>
#include <mutex>
#include <string_view>
#include <thread>
#include "connection.h"
#include "coroutine.h"
#include "http_request.h"
#include "http_response.h"
#include "snowflake.h"

namespace pine
{
  class server;

  /// @brief A connection to a client.
  class server_connection : public connection, public std::enable_shared_from_this<server_connection>
  {
  public:
    /// @brief Construct a server connection with the given socket and server.
    explicit server_connection(SOCKET socket);

    /// @brief Receive an HTTP request.
    /// @param ec An error code to be set if an error occurs.
    /// @return An asynchronous task completed when the request has been received.
    async_operation<http_request> receive_request(std::error_code& ec);

    /// @brief Send an HTTP response.
    /// @param response The response to send.
    /// @param ec An error code to be set if an error occurs.
    /// @return An asynchronous task completed when the response has been sent.
    async_task send_response(http_response const& response, std::error_code& ec);

    /// @brief Start listening for messages from the client.
    /// @return An asynchronous task completed when the connection has been closed.
    async_task start();

  private:
    /// @brief Whether the connection is connected.
    bool is_connected = true;
  };
}