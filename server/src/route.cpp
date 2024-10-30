#include <functional>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <route.h>
#include <route_base.h>
#include <type_traits>
#include <vector>

namespace pine
{
  route::route(route_path path,
               std::function<void(const http_request&,
                                  http_response&)>&&
               handler)
    : route_base(path)
    , handler_(std::forward< std::function<void(const http_request&,
                                                http_response&)>>(handler))

  {}

  route::route(route_path path,
               http_method method,
               std::function<void(const http_request&,
                                  http_response&)>&&
               handler)
    : route_base(path)
    , handler_(std::forward < std::function<void(const http_request&,
                                                 http_response&)>>(handler))
  {
    set_method(method);
  }

  route::route(route_path path,
               std::vector<http_method>&& methods,
               std::function<void(const http_request&,
                                  http_response&)>&&
               handler)
    : route_base(path)
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