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
      response.set_body("Not found");
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
      response.set_body("Not found");
      return;
    }

    response.set_status(http_status::ok);
    response.set_body(read_file(file_location));
  }
}

namespace pine
{
  route_node::route_node(std::string_view path) noexcept
    : path_(path),
    is_path_parameter_(path.starts_with(':'))
  {}


  route_node&
    route_node::add_child(std::string_view path) noexcept
  {
    auto child = std::make_unique<route_node>(path);
    auto child_ptr = child.get();

    children_.push_back(std::move(child));

    if (child_ptr->is_path_parameter_)
    {
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

  route_node&
    route_node::find_child(std::string_view path) const noexcept
  {
    for (auto& child : children_)
    {
      if (path == child->path())
        return *child.get();
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

    return *this;
  }
}
