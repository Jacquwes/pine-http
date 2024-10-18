#include <functional>
#include <string>
#include <type_traits>
#include "http_request.h"
#include "http_response.h"
#include "server_route.h"

namespace pine
{
  server_route::server_route(std::string&& path,
                             std::function<void(const http_request&,
                                                http_response&)>&&
                             handler)
    : path_(std::move(path))
    , handler_(std::move(handler))
  {}
}