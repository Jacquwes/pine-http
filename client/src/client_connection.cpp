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
}