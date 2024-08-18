#pragma once

#include <memory>
#include <mutex>
#include <string_view>
#include <thread>
#include "connection.h"
#include "coroutine.h"
#include "snowflake.h"

namespace pine
{
  class server;

  /// @brief A connection to a client.
  class server_connection : public connection, public std::enable_shared_from_this<server_connection>
  {
  public:
    /// @brief Construct a server connection with the given socket and server.
    explicit server_connection(SOCKET socket);

    /// @brief Start listening for messages from the client.
    /// @return An asynchronous task completed when the connection has been closed.
    async_task start();

  private:
    /// @brief Whether the connection is connected.
    bool is_connected = true;
  };
}