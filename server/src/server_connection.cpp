#include <WinSock2.h>
#include <connection.h>
#include <coroutine.h>
#include <error.h>
#include <loguru.hpp>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <server.h>
#include <server_connection.h>
#include <string>
#include <type_traits>

namespace pine
{
  server_connection::server_connection(SOCKET socket, pine::server& server)
    : connection(socket, server.iocp_)
    , server(server)
  {
    post_read();
  }

  void server_connection::handle_request(http_request& request) const
  {
    const std::string& path = request.get_uri();

    const auto& [route, found, params] = this->server.routes.find_route_with_params(path);

    http_response response;
    response.set_header("Connection", "close");

    if (!found)
    {
      response.set_header("Content-Type", "text/plain");
      response.set_status(http_status::not_found);
      response.set_body("404 Not Found");
    }
    else if (route.handlers()[static_cast<size_t>(request.get_method())] == nullptr)
    {
      response.set_header("Content-Type", "text/plain");
      response.set_status(http_status::method_not_allowed);
      response.set_body("405 Method Not Allowed");
    }
    else
    {
      for (const auto& [name, value] : params)
      {
        request.add_path_param(name, value);
      }

      route.handle(request, response);
    }

    this->send_response(response);
  }

  void server_connection::send_response(http_response const& response) const
  {
    const std::string& response_string = response.to_string();
    post_write(response_string);
  }

  void server_connection::on_write()
  {
    this->close();

    if (auto self = weak_this.lock())
      self->server.remove_client(self->get_socket());
  }


  void server_connection::on_read(std::string_view request_string)
  {
    auto request_result = http_request::parse(std::string(request_string));

    if (!request_result)
    {
      LOG_F(ERROR, "Failed to parse request: %s", request_result.error().message().c_str());
      return;
    }

    auto&& request = std::move(request_result.value());

    handle_request(request);
  }
}
