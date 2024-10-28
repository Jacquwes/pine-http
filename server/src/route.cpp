#include <functional>
#include <string>
#include <type_traits>
#include "http_request.h"
#include "http_response.h"
#include "route.h"

namespace pine
{
  route::route(std::string&& path,
                             std::function<void(const http_request&,
                                                http_response&)>&&
                             handler)
    : path_(std::move(path))
    , handler_(std::move(handler))
  {}

  void route::execute(const http_request& request,
                      http_response& response)
  {
    handler_(request, response);
  }
}