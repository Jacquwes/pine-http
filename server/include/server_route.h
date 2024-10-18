#pragma once

#include <functional>
#include <string>
#include "http_request.h"
#include "http_response.h"

namespace pine
{
  class server_route
  {
  public:
    server_route() = default;
    server_route(std::string&& path,
                 std::function<void(const http_request&,
                                    http_response&)>&&
                 handler);
    ~server_route() = default;

    constexpr const std::string& path() const { return path_; }
    constexpr server_route& set_path(std::string&& path)
    {
      path_ = std::move(path);
      return *this;
    }

    constexpr const std::function<void(const http_request&,
                                       http_response&)>& handler() const
    {
      return handler_;
    }

    inline server_route& set_handler(std::function<void(const http_request&,
                                                        http_response&)>&&
                                     handler)
    {
      handler_ = std::move(handler);
      return *this;
    }

  private:
    std::string path_;
    std::function<void(const http_request&, http_response&)> handler_;
  };
}