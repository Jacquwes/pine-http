#include <coroutine.h>
#include <cstdint>
#include <error.h>
#include <http_request.h>
#include <http_response.h>
#include <string>
#include <system_error>
#include <WinSock2.h>
#include "client_connection.h"
#include "connection.h"

namespace pine
{
  client_connection::client_connection(SOCKET socket)
    : connection{ socket }
  {}

  bool client_connection::connect(std::string const& host, uint16_t const& port)
  {
    // TODO: ???
    if (!connect(host, port))
      return false;

    this->listen();

    return true;
  }

  void client_connection::disconnect()
  {
    this->close();
  }

  async_operation<void>
    client_connection::listen() const
  {
    while (true)
    {
      const auto& received_message_result = co_await receive_raw_message();
      if (!received_message_result)
        co_return received_message_result.error();
    }
  }

  async_operation<http_response>
    client_connection::receive_response() const
  {
    const auto& received_message_result = co_await this->receive_raw_message();
    if (!received_message_result)
      co_return received_message_result.error();

    const auto& response_string = received_message_result.value();

    auto response = http_response::parse(response_string);
    co_return response;
  }

  async_operation<void>
    client_connection::send_request(http_request const& request) const
  {
    const auto& request_string = request.to_string();

    const auto& send_result = co_await this->send_raw_message(request_string);
    if (!send_result)
      co_return send_result.error();

    co_return {};
  }
}