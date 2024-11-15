// Purpose: Source file for server example.

#include <filesystem>
#include <iostream>

#include <loguru.hpp>
#include <server.h>

int main(int argc, char** argv)
{
  loguru::init(argc, argv);
  loguru::g_stderr_verbosity = 1;

  // Create a server on port 27015.
  pine::server server("27015");

  server.add_error_handler(pine::http_status::not_found,
                           [](const pine::http_request&,
                              pine::http_response& response)
                           {
                             response.set_body("This page does not exist.");
                           });

  // Add a route that responds to GET and HEAD requests to the root path.
  // If the path is not valid, the compilation will fail.
  server.add_route("/",
                   [](const pine::http_request&,
                      pine::http_response& response)
                   {
                     response.set_status(pine::http_status::ok);
                     response.set_body("Hello, world!");
                   });

  // Add a route that responds to POST request with a path parameter.
  server.add_route("/:name",
                   [](const pine::http_request& request,
                      pine::http_response& response)
                   {
                     const auto& name = request.get_path_param<std::string>("name");
                     if (!name)
                     {
                       switch (name.error().code())
                       {
                       case pine::error_code::parameter_not_found:
                         response.set_status(pine::http_status::bad_request);
                         response.set_body("The parameter 'name' is required.");
                         return;
                       case pine::error_code::invalid_parameter:
                         response.set_status(pine::http_status::bad_request);
                         response.set_body("The parameter 'name' is invalid.");
                         return;
                       default:
                         response.set_status(pine::http_status::internal_server_error);
                         response.set_body("An error occurred.");
                         return;
                       }
                     }

                     response.set_status(pine::http_status::ok);
                     response.set_body("Hello, " + name.value() + "!");
                   },
                   { pine::http_method::post });

  // Add a route that responds to GET requests to /public_directory/*.
  // This route will serve files from the public directory.
  server.add_static_route("/public_directory", std::filesystem::current_path() / "public");

  // Add a route that responds to GET requests to /public_file.
  // This route will serve the file about.html from the public directory.
  server.add_static_route("/public_file", std::filesystem::current_path() / "public" / "about.html");

  // Start the server
  if (auto server_result = server.start(); !server_result)
  {
    LOG_F(ERROR, "Failed to start server: %s", server_result.error().message().c_str());
    return 1;
  }

  LOG_F(INFO, "Server started on port 27015");

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
