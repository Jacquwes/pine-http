// Purpose: Source file for server example.

#include <filesystem>
#include <iostream>

#include <server.h>

int main()
{
  pine::server server;

  // Add a route that responds with "Hello, world!" to all requests at the root
  server.add_route("/",
                   [](const pine::http_request& request,
                      pine::http_response& response)
                   {
                     response.set_status(pine::http_status::ok);
                     response.set_body("Hello, world!");
                   });

  server.add_static_route("/public_directory", std::filesystem::current_path() / "public");
  server.add_static_route("/public_file", std::filesystem::current_path() / "public" / "about.html");

  if (auto server_result = server.start(); !server_result)
  {
    std::cerr << "Error: " << server_result.error().message() << std::endl;
    return 1;
  }

  std::cout << "Server started." << std::endl;
}
