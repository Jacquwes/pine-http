#include <algorithm>
#include <bit>
#include <cstdint>
#include <ios>
#include <iostream>
#include <random>

#include <connection.h>
#include <coroutine.h>
#include <http_request.h>

#include "server.h"
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

  async_task server_connection::start()
  {
    this->is_connected = true;

    while (is_connected)
    {
    }

    co_return;
  }
}
