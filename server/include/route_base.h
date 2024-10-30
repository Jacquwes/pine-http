#pragma once

#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <route_path.h>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace pine
{
  class route_base
  {
  public:
    route_base() = delete;
    constexpr route_base(route_path path)
    {
      path_ = std::string(path.get());
    }

    virtual ~route_base() = default;
    virtual void execute(const http_request& request,
                         http_response& response) = 0;
    virtual bool matches(const std::string& path) const = 0;

    constexpr const std::string& path() const { return path_; }

  #pragma region Methods
    constexpr const std::vector<http_method>& methods() const
    {
      return methods_;
    }

    constexpr route_base& set_methods(std::vector<http_method>&& methods)
    {
      methods_ = std::move(methods);
      return *this;
    }

    constexpr route_base& set_method(http_method method)
    {
      methods_.clear();
      methods_.push_back(method);
      return *this;
    }

    constexpr route_base& add_method(http_method method)
    {
      methods_.push_back(method);
      return *this;
    }

    constexpr route_base& add_methods(std::vector<http_method>&& methods)
    {
      methods_.insert(methods_.end(),
                      std::make_move_iterator(methods.begin()),
                      std::make_move_iterator(methods.end()));
      return *this;
    }
  #pragma endregion

  protected:
    std::vector<http_method> methods_ = { http_method::get };
    std::string path_;
  };
}