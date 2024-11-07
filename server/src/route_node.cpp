#include <error.h>
#include <expected.h>
#include <http.h>
#include <memory>
#include <route_node.h>
#include <string_view>
#include <type_traits>
#include <vector>

namespace pine
{
  route_node::route_node(std::string_view path) noexcept
    : path_(path),
    handlers_(),
    http_method_mask_(0),
    is_path_parameter_(path.starts_with(':'))
  {}

  std::expected<route_node&, pine::error>
    route_node::add_child(std::string_view path) noexcept
  {
    auto child = std::make_unique<route_node>(path);
    auto child_ptr = child.get();

    children_.push_back(std::move(child));

    if (child_ptr->is_path_parameter_)
    {
      if (has_path_parameter_children_)
        return
        std::make_unexpected(pine::error(error_code::path_parameter_conflict));

      has_path_parameter_children_ = true;
      path_parameter_child_ = child_ptr;
    }

    return *child_ptr;
  }

  void route_node::add_handler(http_method method, std::unique_ptr<handler_type> handler) noexcept
  {
    handlers_[static_cast<size_t>(method)] = std::move(handler);
    http_method_mask_ |= 1 << static_cast<size_t>(method);
  }

  std::expected<route_node&, pine::error>
    route_node::find_child(std::string_view path) const noexcept
  {
    for (auto& child : children_)
    {
      if (path.starts_with(child->path()))
        return *child.get();
    }

    if (has_path_parameter_children_)
      return *path_parameter_child_;

    return std::make_unexpected(pine::error(error_code::route_not_found));
  }
}
