#include <WinSock2.h>
#include <connection.h>
#include <coroutine.h>
#include <error.h>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <route.h>
#include <server.h>
#include <server_connection.h>
#include <string>

namespace pine
{
  server_connection::server_connection(SOCKET socket, pine::server& server)
    : connection(socket)
    , server(server)
  {}

  async_operation<void>
    server_connection::handle_request(const http_request& request)
  {
    const std::string& path = request.get_uri();

    const auto& route_result =
      this->server.get_route(path, request.get_method());

    http_response response;
    response.set_header("Connection", "close");

    if (!route_result)
      switch (route_result.error().code())
      {
      case error_code::route_not_found:
        response.set_header("Content-Type", "text/plain");
        response.set_status(http_status::not_found);
        response.set_body("404 Not Found");
        break;
      case error_code::method_not_allowed:
        response.set_header("Content-Type", "text/plain");
        response.set_status(http_status::method_not_allowed);
        response.set_body("405 Method Not Allowed");
        break;
      default:
        response.set_header("Content-Type", "text/plain");
        response.set_status(http_status::internal_server_error);
        response.set_body("500 Internal Server Error");
        break;
      }
    else
      route_result.value()->execute(request, response);

    co_return co_await this->send_response(response);
  }

  async_operation<http_request>
    pine::server_connection::receive_request() const
  {
    const auto& receive_message_result = co_await this->receive_raw_message();
    if (!receive_message_result)
      co_return receive_message_result.error();

    const std::string& request_string = receive_message_result.value();

    const auto& request_result = http_request::parse(request_string);
    if (!request_result)
      co_return request_result.error();

    co_return request_result.value();
  }

  async_operation<void>
    server_connection::send_response(http_response const& response) const
  {
    const std::string& response_string = response.to_string();
    const auto& send_message_result =
      co_await this->send_raw_message(response_string);

    if (!send_message_result)
      co_return send_message_result.error();

    co_return{};
  }

  async_operation<void>
    server_connection::start()
  {
    this->is_connected = true;

    const auto& request_result = co_await this->receive_request();
    if (!request_result)
    {
      this->is_connected = false;
      co_return request_result.error();
    }

    const auto& request = request_result.value();

    const auto& response_result = co_await this->handle_request(request);

    this->close();
    this->is_connected = false;

    if (!response_result)
      co_return response_result.error();
    co_return{};
  }
}
