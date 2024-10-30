// Purpose: Source file for server example.

#include <filesystem>
#include <iostream>

#include <server.h>

int main()
{
  pine::server server;

  // Add a route that responds to GET and HEAD requests to the root path.
  // If the path is not valid, the compilation will fail.
  server.add_route("/",
                   [](const pine::http_request& request,
                      pine::http_response& response)
                   {
                     response.set_status(pine::http_status::ok);
                     response.set_body("Hello, world!");
                   })
    .set_methods({ pine::http_method::get, pine::http_method::head });

  // Add a route that responds to POST requests to the root path.
  server.add_route("/",
                   pine::http_method::post,
                   [](const pine::http_request& request,
                      pine::http_response& response)
                   {
                     response.set_status(pine::http_status::ok);
                     response.set_body("Hello, post!");
                   });


  // Add a route that responds to GET requests to /public_directory/*.
  // This route will serve files from the public directory.
  server.add_static_route("/public_directory", std::filesystem::current_path() / "public");

  // Add a route that responds to GET requests to /public_file.
  // This route will serve the file about.html from the public directory.
  server.add_static_route("/public_file", std::filesystem::current_path() / "public" / "about.html");

  // Start the server
  if (auto server_result = server.start(); !server_result)
  {
    std::cerr << "Error: " << server_result.error().message() << std::endl;
    return 1;
  }

  std::cout << "Server started." << std::endl;
}
