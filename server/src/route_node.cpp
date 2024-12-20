#include <error.h>
#include <expected.h>
#include <fstream>
#include <http.h>
#include <iterator>
#include <memory>
#include <route_node.h>
#include <route_path.h>
#include <route_tree.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

/// @brief Read the file from the location and return the contents as a string.
/// @param location Location of the file.
/// @return The contents of the file as a string.
static std::string read_file(const std::filesystem::path& location)
{
  std::ifstream file(location, std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

/// @brief Get the file path from the request URI
/// @param request_path The path of the route
/// @param requested_uri The requested URI
/// @param location The location of the file/directory
/// @return The file path
static std::filesystem::path get_file_path(std::string_view request_path,
                                           std::string_view requested_uri,
                                           const std::filesystem::path& location)
{
  std::string_view file_path = requested_uri.substr(request_path.size());
  if (file_path.size() == 0)
    return "index.html";

  if (file_path.starts_with('/'))
    file_path = file_path.substr(1);

  return location / file_path;
}

namespace pine
{
  static void serve_files(std::string_view route_path,
                          const http_request& request,
                          http_response& response,
                          const std::filesystem::path& location)
  {
    if (!std::filesystem::exists(location))
    {
      response.set_status(http_status::not_found);
      response.set_body("404 Not found");
      return;
    }

    if (!std::filesystem::is_directory(location))
    {
      response.set_status(http_status::ok);
      response.set_body(read_file(location));
      return;
    }

    std::filesystem::path file_location = get_file_path(route_path,
                                                        request.get_uri(),
                                                        location);
    if (!std::filesystem::exists(file_location))
    {
      response.set_status(http_status::not_found);
      response.set_body("404 Not found");
      return;
    }

    response.set_status(http_status::ok);
    response.set_body(read_file(file_location));
  }
}

namespace pine
{
  route_node::route_node(std::string_view path)
    : path_(path),
    is_path_parameter_(path.starts_with(':'))
  {
    if (path_ == "/")
    {
      path_ = "";
      return;
    }

    for (size_t i = 0; i < path.size(); i++)
    {
      if (path[i] == '/')
      {
        // construct child with the rest of the path
        path_ = path.substr(0, i);
        add_child(path.substr(i + 1));
        break;
      }
    }

  }

  route_node&
    route_node::add_child(std::string_view path)
  {
    auto child = std::make_unique<route_node>(path);
    auto child_ptr = child.get();

    children_.push_back(std::move(child));

    if (child_ptr->is_path_parameter_)
    {
      if (has_path_parameter_children_)
        throw error(error_code::path_parameter_conflict);

      has_path_parameter_children_ = true;
      path_parameter_child_ = child_ptr;
    }

    return *child_ptr;
  }

  void route_node::add_handler(http_method method,
                               std::unique_ptr<handler_type> handler) noexcept
  {
    handlers_[static_cast<size_t>(method)] = std::move(handler);
    http_method_mask_ |= 1 << static_cast<size_t>(method);
  }

  static bool paths_match(std::string_view node_path, std::string_view path)
  {
    // node_path: api
    // path:      api/users/
    // should return true, because api is a prefix of api/users
    size_t child_length = node_path.size();
    size_t path_length = path.size();
    size_t min_length = std::min(child_length, path_length);

    size_t i;
    for (i = 0; i < min_length; i++)
    {
      if (path[i] != '/' && path[i] != node_path[i])
        return false;

      if (path[i] == '/')
      {
        if (i == child_length)
          return true;

        return false;
      }
    }

    // check if the path is a prefix of the node path
    if (child_length > path_length)
      return false;

    if (child_length == path_length)
      return true;

    return path[i] == '/';
  }

  route_node&
    route_node::find_child(std::string_view path) const noexcept
  {
    for (const auto& child : children_)
    {
      if (paths_match(child->path_, path))
        return *child;
    }

    if (has_path_parameter_children_)
      return *path_parameter_child_;

    return route_tree::unknown_route;
  }

  route_node& route_node::serve_files(std::filesystem::path&& location)
  {
    handlers_[static_cast<size_t>(http_method::get)] =
      std::make_unique<handler_type>(
        [this, location = std::move(location)](const http_request& request,
                                               http_response& response)
        {
          pine::serve_files(path_, request, response, location);
        });

    http_method_mask_ |= 1 << static_cast<size_t>(http_method::get);

    return *this;
  }
}
