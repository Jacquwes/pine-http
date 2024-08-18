// Purpose: Source file for server example.

#include <iostream>

#include <server.h>

int main()
{
  pine::server server;
  std::error_code ec;
  server.start(ec);
  if (ec)
  {
    std::cerr << "Error: " << ec.message() << std::endl;
    return 1;
  }

  std::cout << "Server started." << std::endl;
}