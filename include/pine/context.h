#pragma once

namespace pine
{
  struct connection;

  struct accept_context;

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
