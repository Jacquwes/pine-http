#pragma once

#include <atomic>
#include <connection.h>
#include <http_request.h>
#include <http_response.h>
#include <memory>

namespace pine
{
  class server;

  /// @brief A connection to a client.
  template <size_t buffer_size>
  class server_connection
    : public connection<buffer_size>,
    public std::enable_shared_from_this<server_connection<buffer_size>>
  {
    friend class server;

  public:
    /// @brief Construct a server connection with the given socket and server.
    explicit server_connection(SOCKET socket, pine::server& server)
      : connection<buffer_size>(socket, server.iocp_),
      server(server)
    {}

    /// @brief Close the connection and remove it from the server.
    void close() override
    {
      if (pending_close.exchange(true))
      {
        return;
      }

      std::lock_guard write_lock{ this->write_mutex };
      std::lock_guard read_lock{ this->read_mutex };

      connection<buffer_size>::close();

      auto self = server_connection<buffer_size>::shared_from_this();
      server.remove_client(connection<buffer_size>::get_socket());
    }

    /// @brief Handle an error. This function will modify the response to
    /// contain the appropriate error message.
    /// @param status The status of the error.
    /// @param request The request that caused the error.
    /// @param response The response.
    void handle_error(http_status status,
                      const http_request& request,
                      http_response& response) const
    {
      const auto& handler = server.error_handlers[status];

      response.set_header("Connection", "close");
      response.set_status(status);

      handler(request, response);
    }

    /// @brief Handle a request. This function will route the request to the
    /// appropriate handler and send the response.
    /// @param request The request to handle.
    void handle_request(http_request& request)
    {
      const std::string_view& path = request.get_uri();

      const auto& [route, found, params] =
        server.routes.find_route_with_params(path);

      http_response response;
      // Keep alive is not supported yet.
      response.set_header("Connection", "close");

      if (!found)
        handle_error(http_status::not_found, request, response);
      else if (route.handlers()[static_cast<size_t>(request.get_method())] == nullptr)
        handle_error(http_status::method_not_allowed, request, response);
      else
      {
        for (const auto& [name, value] : params)
          request.add_path_param(name, value);
        route.handle(request, response);
      }

      send_response(response);
    }

    /// @brief Handle a read operation.
    /// @param data The data to read.
    void on_read(std::string_view message) override
    {
      auto self = server_connection<buffer_size>::shared_from_this();
      auto request_result = http_request::parse(message);
      if (!request_result)
      {
        http_response response;
        http_request request;
        handle_error(http_status::bad_request, request, response);
        send_response(response);
        return;
      }
      auto&& request = std::move(request_result.value());
      handle_request(request);
    }

    /// @brief Handle a write operation.
    /// @param data The data to write.
    void on_write() override
    {
      auto self =
        server_connection<buffer_size>::shared_from_this();

      // The response has been sent, so close the connection.
      if (!this->write_pending)
        close();
    }

    /// @brief Send an HTTP response.
    /// @param response The response to send.
    /// @return An asynchronous task completed when the response has been sent.
    void send_response(http_response const& response)
    {
      auto self = server_connection<buffer_size>::shared_from_this();
      std::string response_string = response.to_string();
      connection<buffer_size>::post_write(response_string);
    }

  private:
    /// @brief The server that the connection is connected to.
    server& server;

    /// @brief Whether the connection is pending close.
    std::atomic_bool pending_close = false;
  };
}
