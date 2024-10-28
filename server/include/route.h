#pragma once

#include <functional>
#include <http_request.h>
#include <http_response.h>
#include <route_base.h>
#include <string>
#include <type_traits>

namespace pine
{
  class route : public route_base
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

    void execute(const http_request& request,
                 http_response& response) override;

    constexpr const std::function<void(const http_request&,
                                       http_response&)>& handler() const
    {
      return handler_;
    }

    bool matches(const std::string& path) const override
    {
      return path_ == path;
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