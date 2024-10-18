#include <connection.h>
#include <coroutine.h>
#include <http_request.h>
#include <http_response.h>
#include <string>
#include <system_error>
#include <WinSock2.h>
#include <error.h>
#include "server.h"
#include "server_connection.h"

namespace pine
{
  server_connection::server_connection(SOCKET socket, pine::server& server)
    : connection(socket)
    , server(server)
  {}

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

    co_return error(error_code::success);
  }

  async_operation<void>
    server_connection::start()
  {
    this->is_connected = true;

    while (is_connected)
    {
      const auto& request_result = co_await this->receive_request();
      if (!request_result)
      {
        this->is_connected = false;
        co_return request_result.error();
      }
    }

    this->is_connected = false;
    co_return error(error_code::success);
  }
}
