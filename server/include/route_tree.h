#pragma once

#include <error.h>
#include <expected.h>
#include <functional>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <initializer_list>
#include <memory>
#include <route_node.h>
#include <route_path.h>
#include <string_view>

namespace pine
{
  class route_tree
  {
  public:
    using handler_type =
      std::function<void(const http_request&, http_response&)>;

    route_tree() = default;

    /// @brief Adds a route to the tree. 
    /// 
    /// The route is represented by a path, a method and a handler. 
    /// 
    /// A route can have paths parameters, which are represented by a
    /// colon followed by the parameter name. For example, "/users/:id" is a
    /// route with a path parameter named "id". The handler is called with the
    /// path parameters in the request.
    /// 
    /// @param path The path of the route.
    /// @param handler The handler of the route.
    /// @param methods The HTTP methods that the route supports.
    std::expected<void, error>
      add_route(const route_path& path,
                const std::function<void(const http_request&,
                                         http_response&)>& handler,
                std::initializer_list<http_method> methods
                = { http_method::get }) const;

    /// @brief Finds a route in the tree.
    /// @param path The path to search for.
    /// @param method The HTTP method to search for.
    /// @return A shared pointer to the route node if found, otherwise nullptr.
    std::expected<route_node&, error> find_route(std::string_view path,
                                                 http_method method) const;

  private:
    std::unique_ptr<route_node> root_ = std::make_unique<route_node>("/");
  };
}
