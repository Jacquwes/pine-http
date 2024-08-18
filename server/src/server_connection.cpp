#include <algorithm>
#include <bit>
#include <cstdint>
#include <ios>
#include <iostream>
#include <random>

#include <connection.h>
#include <coroutine.h>
#include <snowflake.h>

#include "server.h"
#include "server_connection.h"


namespace pine
{
  server_connection::server_connection(SOCKET socket) : connection(socket)
  {}

  async_task server_connection::start()
  {
    this->is_connected = true;

    while (is_connected)
    {
    }

    co_return;
  }
}
