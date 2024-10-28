#pragma once

#include <functional>
#include <string>
#include "http_request.h"
#include "http_response.h"

namespace pine
{
  class route
  {
  public:
    route() = default;
    route(std::string&& path,
                 std::function<void(const http_request&,
                                    http_response&)>&&
                 handler);
    ~route() = default;

    constexpr const std::string& path() const { return path_; }
    constexpr route& set_path(std::string&& path)
    {
      path_ = std::move(path);
      return *this;
    }

    constexpr const std::function<void(const http_request&,
                                       http_response&)>& handler() const
    {
      return handler_;
    }

    inline route& set_handler(std::function<void(const http_request&,
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