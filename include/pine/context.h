#pragma once

namespace pine
{
  struct connection;
  struct server;

  struct accept_context
  {
    server* server;
    socket client_socket;
  };

  struct read_context
  {
    connection* connection;
    const char* buffer;
    size_t size;
  };

  struct write_context
  {
    connection* connection;
    const char* buffer;
    size_t size;
  };
}
