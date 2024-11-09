#include <expected.h>
#include <http.h>
#include <memory>
#include <route_node.h>
#include <route_path.h>
#include <route_tree.h>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace pine
{
  route_node route_tree::unknown_route("");

  route_node& route_tree::add_route(const route_path& path)
  {
    auto [match, depth, deepest_node] = get_deepest_node(path);
    if (match)
      return deepest_node;

    auto node = &deepest_node;
    const auto& parts = path.parts();

    for (size_t i = depth; i < parts.size(); i++)
    {
      node = &node->add_child(parts[i]);
    }

    return *node;
  }

  const route_node&
    route_tree::find_route(std::string_view path) const
  {
    auto node = root_.get();

    if (path == "/")
      return *node;

    path.remove_prefix(1);

    for (size_t i = 0; i < path.size();)
    {
      auto child = &node->find_child(path.substr(i));
      if (child == &unknown_route)
        return unknown_route;

      node = child;
      i += node->path().size() + 1;
    }

    return *node;
  }

  std::tuple<const route_node&,
    bool,
    std::unordered_map<std::string, std::string_view>>
    route_tree::find_route_with_params(std::string_view path) const
  {
    auto node = root_.get();
    std::unordered_map<std::string, std::string_view> params;

    if (path == "/")
      return { *node, true, params };

    path.remove_prefix(1);

    for (size_t i = 0; i < path.size();)
    {
      auto child = &node->find_child(path.substr(i));
      if (child == &unknown_route)
        return { *node, false, params };

      if (child->is_path_parameter())
      {
        std::string_view param_name = child->path().substr(1);
        size_t end = path.find_first_of("/", i);
        if (end == std::string_view::npos)
          end = path.size();

        params[std::string(param_name)] = path.substr(i, end - i);

        i = end + 1;
      }
      else
        i += child->path().size() + 1;

      node = child;
    }

    return { *node, true, params };
  }

  std::tuple<bool, size_t, route_node&>
    route_tree::get_deepest_node(std::string_view path) const
  {
    auto node = root_.get();
    size_t depth = 0;

    if (path == "/")
      return { true, depth, *node };

    path.remove_prefix(1);

    for (size_t i = 0; i < path.size();)
    {
      auto child = &node->find_child(path.substr(i));
      if (child == &unknown_route)
        return { false, depth, *node };

      node = child;
      i += node->path().size() + 1;
      depth++;
    }

    return { true, depth, *node };
  }
}
