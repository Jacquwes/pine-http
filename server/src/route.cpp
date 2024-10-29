#include <functional>
#include <http.h>
#include <route_base.h>
#include <string>
#include <type_traits>
#include <vector>
#include <http_request.h>
#include <http_response.h>
#include <route.h>

namespace pine
{
  route::route(std::string&& path,
               std::function<void(const http_request&,
                                  http_response&)>&&
               handler)
    : route_base(std::forward<std::string>(path))
    , handler_(std::forward< std::function<void(const http_request&,
                                                http_response&)>>(handler))
  {}

  route::route(std::string&& path,
               http_method method,
               std::function<void(const http_request&,
                                  http_response&)>&&
               handler)
    : route_base(std::forward < std::string>(path))
    , handler_(std::forward < std::function<void(const http_request&,
                                                 http_response&)>>(handler))
  {
    set_method(method);
  }

  route::route(std::string&& path,
               std::vector<http_method>&& methods,
               std::function<void(const http_request&,
                                  http_response&)>&&
               handler)
    : route_base(std::forward<std::string>(path))
    , handler_(std::forward < std::function<void(const http_request&,
                                                 http_response&)>>(handler))
  {
    set_methods(std::forward<std::vector<http_method>>(methods));
  }

  void route::execute(const http_request& request,
                      http_response& response)
  {
    handler_(request, response);
  }
}