#pragma once

#include <http_request.h>
#include <http_response.h>
#include <string>

namespace pine
{
  class route_base
  {
  public:
    virtual ~route_base() = default;
    virtual void execute(const http_request& request,
                         http_response& response) = 0;
    virtual const std::string& path() const = 0;
  };
}