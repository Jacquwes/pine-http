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
#include <route_tree.h>

namespace pine
{
  std::expected<void, error>
    route_tree::add_route(const route_path& path,
                          const std::function<void(const http_request&,
                                                   http_response&)>& handler,
                          std::initializer_list<http_method> methods)
    const
  {
    auto node = root_.get();

    for (const auto& part : path.parts())
    {
      route_node* child = nullptr;

      auto child_result = node->find_child(part);
      if (!child_result)
      {
        auto add_result = node->add_child(part);
        if (!add_result)
          return std::make_unexpected(add_result.error());
        child = &add_result.value();
      }
      else
        child = &child_result.value();

      node = child;
    }

    for (auto method : methods)
      node->add_handler(method,
                        std::make_unique<route_node::handler_type>(handler));
  }

  std::expected<route_node&, error>
    route_tree::find_route(std::string_view path, http_method method) const
  {
    auto node = root_.get();

    for (size_t i = 0; i < path.size();)
    {
      auto child_result = node->find_child(path.substr(i));
      if (!child_result)
        return (child_result);

      node = &child_result.value();
      i += node->path().size();
    }

    if (node->handlers()[static_cast<size_t>(method)] != nullptr)
      return *node;

    return std::make_unexpected(error(error_code::route_not_found));
  }
}
