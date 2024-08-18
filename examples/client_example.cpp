#include <iostream>

#include <client.h>

int main()
{
  pine::client client;

  if (client.connect("localhost"))
  {
    std::cout << "Connected to server" << std::endl;
  }
  else
  {
    std::cout << "Failed to connect to server" << std::endl;
    return 1;
  }
}