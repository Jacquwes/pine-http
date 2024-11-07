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

namespace pine
{
  /// @brief A node in a radix tree that represents a route or a part of a 
  /// route.
  class route_node
  {
  public:
    using handler_type =
      std::function<void(const http_request&, http_response&)>;

  public:
    explicit(false) route_node(std::string_view path) noexcept;

    constexpr
      std::string_view path() const noexcept { return path_; }

    constexpr
      const std::array<std::unique_ptr<handler_type>, http_method_count>&
      handlers() const noexcept { return handlers_; }

    std::expected<route_node&, pine::error>
      add_child(std::string_view path) noexcept;

    void add_handler(http_method method,
                     std::unique_ptr<handler_type> handler) noexcept;

    std::expected<route_node&, pine::error>
      find_child(std::string_view path) const noexcept;

  private:
    // Optimization: store path alongside the children.
    // Optimization: Maybe make a small cache for the children?

    std::vector<std::unique_ptr<route_node>> children_;
    std::string path_;
    std::array<std::unique_ptr<handler_type>, http_method_count> handlers_;
    uint16_t http_method_mask_ = 0;

    bool is_path_parameter_ = false;
    bool has_path_parameter_children_ = false;
    route_node* path_parameter_child_ = nullptr;
  };
}