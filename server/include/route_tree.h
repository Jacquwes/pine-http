#pragma once

#include <functional>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <memory>
#include <route_node.h>
#include <route_path.h>
#include <string_view>
#include <utility>

namespace pine
{
  /// @brief A tree that represents routes.
  class route_tree
  {
  public:
    /// @brief The type of the handler function.
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
    route_node& add_route(const route_path& path);

    /// @brief Finds a route in the tree.
    /// @param path The path to search for.
    /// @param method The HTTP method to search for.
    /// @return A reference to the route node if found; otherwise, a reference to
    /// the unknown route node.
    const route_node& find_route(std::string_view path) const;

    /// @brief Gets the root node of the tree.
    /// @return A reference to the root node.
    route_node& root() noexcept { return *root_; }

    static route_node unknown_route;

  private:
    /// @brief Gets the deepest node in the tree that matches the path.
    /// @param path The path to search for.
    /// @return A tuple with a boolean indicating if the node was found, a
    /// size_t indicating the depth of the node in the tree, and a reference to
    /// the deepest corresponding node.
    /// reference to the node.
    std::tuple<bool, size_t, route_node&>
      get_deepest_node(std::string_view path) const;

    std::unique_ptr<route_node> root_ = std::make_unique<route_node>("/");
  };
}
