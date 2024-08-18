#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include "connection.h"
#include "coroutine.h"
#include "http_request.h"
#include "http_response.h"

namespace pine
{
  class client;

  class client_connection : public connection
  {
  public:
    /// @brief Construct a client connection.
    /// @param socket The socket to use for the connection.
    explicit client_connection(SOCKET socket);

    /// @brief Connect to a server.
    /// @param host Host name or IP address of the server.
    /// @param port TCP port of the server.
    /// @return True if the connection was successful, false otherwise.
    bool connect(std::string const& host, uint16_t const& port = 80);

    /// @brief Disconnect from the server.
    void disconnect();

    /// @brief Start listening for messages from the server.
    /// @return An asynchronous task completed when the connection has been closed.
    async_task listen();

    /// @brief Receive an HTTP response from the server.
    /// @param ec The error code set if the response could not be received.
    /// @return An asynchronous task completed when the response has been received.
    async_operation<http_response> receive_response(std::error_code& ec);

    /// @brief Send an HTTP request to the server.
    /// @param request The request to send.
    /// @param ec The error code set if the request could not be sent.
    /// @return An asynchronous task completed when the request has been sent.
    async_task send_request(http_request const& request, std::error_code& ec);

  private:
    std::jthread client_thread;
  };
}