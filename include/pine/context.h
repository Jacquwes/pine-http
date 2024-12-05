#pragma once

#include <pine/socket.h>

namespace pine
{
  struct connection;
  struct server;

  struct accept_context
  {
    pine::server* server;
    socket client_socket;
  };

  struct read_context
  {
    pine::connection* connection;
    const char* buffer;
    size_t size;
  };

  struct write_context
  {
    pine::connection* connection;
    const char* buffer;
    size_t size;
  };
}
