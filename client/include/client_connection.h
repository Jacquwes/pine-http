#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include "connection.h"
#include "coroutine.h"

namespace pine
{
  class client;

  class client_connection : public connection
  {
  public:
    /// @brief Construct a client connection.
    /// @param socket The socket to use for the connection.
    explicit client_connection(SOCKET socket);

    /// @brief Connect to a server.
    /// @param host Host name or IP address of the server.
    /// @param port TCP port of the server.
    /// @return True if the connection was successful, false otherwise.
    bool connect(std::string const& host, uint16_t const& port = 80);

    /// @brief Disconnect from the server.
    void disconnect();

    /// @brief Start listening for messages from the server.
    /// @return An asynchronous task completed when the connection has been closed.
    async_task listen();

  private:
    std::jthread client_thread;
  };
}