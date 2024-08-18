#include <connection.h>
#include <coroutine.h>
#include <http_request.h>
#include <http_response.h>
#include <string>
#include <system_error>
#include <WinSock2.h>
#include "server_connection.h"

namespace pine
{
  server_connection::server_connection(SOCKET socket) : connection(socket)
  {}

  async_operation<http_request> pine::server_connection::receive_request(std::error_code& ec)
  {
    std::string request_string = co_await this->receive_raw_message(ec);
    if (ec)
      co_return http_request();

    auto request = http_request::parse(request_string, ec);

    co_return request;
  }

  async_task server_connection::send_response(http_response const& response, std::error_code& ec)
  {
    std::string response_string = response.to_string();
    co_await this->send_raw_message(response_string, ec);
  }

  async_task server_connection::start(std::error_code& ec)
  {
    this->is_connected = true;

    while (is_connected)
    {
      const http_request& request = co_await this->receive_request(ec);
      if (ec)
        break;
    }

    co_return;
  }
}
