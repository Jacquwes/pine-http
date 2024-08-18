#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include "coroutine.h"
#include "client.h"
#include "client_connection.h"

namespace pine
{
  bool client::connect(std::string const& host, uint16_t const& port)
  {
    if (!this->connection->connect(host, port))
      return false;

    // TODO: Async
    this->connection->listen();

    return true;
  }

  void client::disconnect()
  {
    this->connection->disconnect();

    std::cout << "[Client] Disconnected from server" << std::endl;
  }
}
