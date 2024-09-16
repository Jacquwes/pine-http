// Purpose: Source file for server example.

#include <iostream>

#include <server.h>

int main()
{
  pine::server server;

  if (auto server_result = server.start(); !server_result)
  {
    std::cerr << "Error: " << server_result.error().message() << std::endl;
    return 1;
  }

  std::cout << "Server started." << std::endl;
}
