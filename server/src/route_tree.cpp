#include <expected.h>
#include <http.h>
#include <memory>
#include <route_node.h>
#include <route_path.h>
#include <route_tree.h>
#include <string_view>
#include <utility>

namespace pine
{
  route_node route_tree::unknown_route("");

  route_node& route_tree::add_route(const route_path& path)
  {
    auto [match, deepest_node] = get_deepest_node(path);
    if (match)
      return deepest_node;

    auto node = &deepest_node;
    const auto& parts = path.parts();

    for (const auto& part : parts)
      node = &node->add_child(part);

    return *node;
  }

  const route_node&
    route_tree::find_route(std::string_view path) const
  {
    auto node = root_.get();

    if (path == "/")
      return *node;

    for (size_t i = 0; i < path.size();)
    {
      auto child = &node->find_child(path.substr(i));
      if (child == &unknown_route)
        return unknown_route;

      node = child;
      i += node->path().size();
    }

    return *node;
  }

  std::pair<bool, route_node&>
    route_tree::get_deepest_node(std::string_view path) const
  {
    auto node = root_.get();

    for (size_t i = 0; i < path.size();)
    {
      auto child = &node->find_child(path.substr(i));
      if (child == &unknown_route)
        return { false, *node };

      node = child;
      i += node->path().size();
    }

    return { true, *node };
  }
}
