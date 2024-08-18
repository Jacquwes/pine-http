#include <cstdint>
#include <string>
#include "client_connection.h"
#include "client.h"
#include "connection.h"

namespace pine
{
  client_connection::client_connection(SOCKET socket)
    : connection{ socket }
  {}

  bool client_connection::connect(std::string const& host, uint16_t const& port)
  {
    if (!this->connect(host, port))
      return false;

    this->listen();

    return true;
  }

  void client_connection::disconnect()
  {
    this->close();
  }

  async_task client_connection::listen()
  {
    while (true)
    {
      std::error_code ec;
      co_await receive_raw_message(ec);
    }
  }

  async_operation<http_response> client_connection::receive_response(std::error_code& ec)
  {
    std::string response_string = co_await this->receive_raw_message(ec);
    if (ec)
      co_return http_response{};

    auto response = http_response::parse(response_string, ec);
    co_return response;
  }

  async_task client_connection::send_request(http_request const& request, std::error_code& ec)
  {
    auto request_string = request.to_string();
    co_await this->send_raw_message(request_string, ec);
  }
}