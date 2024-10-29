#pragma once

#include <http_request.h>
#include <http_response.h>
#include <string>

namespace pine
{
  class route_base
  {
  public:
    route_base() = delete;
    constexpr route_base(std::string&& path)
      : path_(std::forward<std::string>(path))
    {}

    virtual ~route_base() = default;
    virtual void execute(const http_request& request,
                         http_response& response) = 0;
    virtual bool matches(const std::string& path) const = 0;

    constexpr const std::string& path() const { return path_; }
  protected:
    std::string path_;
  };
}