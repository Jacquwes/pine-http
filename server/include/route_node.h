#pragma once

#include <array>
#include <cstdint>
#include <error.h>
#include <expected.h>
#include <functional>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>

namespace pine
{
  /// @brief A node in a radix tree that represents a route or a part of a 
  /// route.
  class route_node
  {
  public:

    /// @brief The type of the handler function.
    using handler_type =
      std::function<void(const http_request&, http_response&)>;

    /// @brief Construct a new base route node. The path of the node
    /// corresponds to one part of a route (e.g. a segment of the URI).
    /// 
    /// If the path contains a colon followed by a name (e.g. "/users/:id"),
    /// the node will be a path parameter node. Path parameter nodes are used to
    /// represent routes with path parameters. The path parameter name is the
    /// name following the colon, and can be used to extract the path parameter
    /// with the pine::http_request::get_path_param<T>(std::string_view name)
    /// function.
    /// 
    /// @param path The path of the node.
    /// @return A new base route node.
    explicit(false) route_node(std::string_view path) noexcept;

    void handle(const http_request& request,
                http_response& response) const noexcept
    {
      handlers_[static_cast<size_t>(request.get_method())]->operator()(request, response);
    }

    /// @brief Get the path of the node. The path of the node corresponds to one
    /// part of a route (e.g. a segment of the URI).
    /// @return The path of the node.
    constexpr std::string_view path() const noexcept { return path_; }

    /// @brief Get the handlers of the node, indexed by pine::http_method.
    /// @return The handlers of the node.
    constexpr
      const std::array<std::unique_ptr<handler_type>, http_method_count>&
      handlers() const noexcept { return handlers_; }

    /// @brief Add a child to the node. The child will be a part of the route
    /// that the node represents.
    /// @param path The path of the child.
    /// @return A reference to the child. If the child already exists, a
    /// reference to the existing child.
    route_node&
      add_child(std::string_view path);

    /// @brief Add a handler to the node. The handler will be called when the
    /// route represented by the node is requested and the method matches.
    /// Calling this function will overwrite any existing handler for the method.
    /// @param method The HTTP method to handle.
    /// @param handler The handler to call.
    void add_handler(http_method method,
                     std::unique_ptr<handler_type> handler) noexcept;

    /// @brief Find a child of the node by path.
    /// @param path The path of the child to find. The path can be a segment of
    /// the URI or the rest of the URI.
    /// @return If the child was found, a reference to the child. If the child
    /// was not found, an error code.
    route_node&
      find_child(std::string_view path) const noexcept;

    /// @brief Check if the node is a path parameter node.
    /// @return True if the node is a path parameter node, otherwise false.
    constexpr bool is_path_parameter() const noexcept
    {
      return is_path_parameter_;
    }

    /// @brief Check if the node has children that are path parameter nodes.
    /// @return True if the node has children that are path parameter nodes,
    constexpr bool has_path_parameter_children() const noexcept
    {
      return has_path_parameter_children_;
    }

    /// @brief Get the list of children of the node.
    /// @return The list of children of the node.
    constexpr const std::vector<std::unique_ptr<route_node>>& children() const noexcept
    {
      return children_;
    }

    route_node& serve_files(std::filesystem::path&& location);

  private:
    // Optimization: Store the start of path alongside the children for faster
    // comparison.
    // Optimization: Maybe make a small cache for the children?
    // Optimization: Store whether the node is an endpoint or not.

    std::array<std::unique_ptr<handler_type>, http_method_count> handlers_{};
    uint16_t http_method_mask_ = 0;

    std::vector<std::unique_ptr<route_node>> children_;
    std::string path_;

    bool is_path_parameter_ = false;
    bool has_path_parameter_children_ = false;
    route_node* path_parameter_child_ = nullptr;
  };
}