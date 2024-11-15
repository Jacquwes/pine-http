#pragma once

#include <connection.h>
#include <coroutine.h>
#include <http_request.h>
#include <http_response.h>
#include <iocp.h>
#include <memory>
#include <system_error>

namespace pine
{
  class server;

  /// @brief A connection to a client.
  class server_connection
    : public connection
    , public std::enable_shared_from_this<server_connection>
  {
    friend class server;

  public:
    /// @brief Construct a server connection with the given socket and server.
    explicit server_connection(SOCKET socket, pine::server& server);

    void handle_request(http_request& request) const;

    /// @brief Handle a read operation.
    /// @param data The data to read.
    void on_read(std::string_view message) override;

    /// @brief Handle a write operation.
    /// @param data The data to write.
    void on_write() override;

    /// @brief Send an HTTP response.
    /// @param response The response to send.
    /// @return An asynchronous task completed when the response has been sent.
    void send_response(http_response const& response) const;

    /// @brief Start listening for messages from the client.
    /// @return An asynchronous task completed when the connection has been closed.
    void start();

  private:
    /// @brief The server that the connection is connected to.
    server& server;

    std::weak_ptr<server_connection> weak_this;
  };
}