#include <filesystem>
#include <fstream>
#include <http.h>
#include <http_request.h>
#include <http_response.h>
#include <iterator>
#include <static_route.h>
#include <string>

// Read the file from the location and return the contents as a string.
static std::string read_file(const std::filesystem::path& location)
{
  std::ifstream file(location, std::ios::binary);
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

// Get the file path from the request URI
static std::filesystem::path get_file_path(const std::string& request_path,
                                           const std::string& requested_uri,
                                           const std::filesystem::path& location)
{
  std::string file_path = requested_uri.substr(request_path.size());
  if (file_path.size() == 0)
    return "index.html";

  if (file_path.starts_with('/'))
    file_path = file_path.erase(0, 1);

  return location / file_path;
}

namespace pine
{
  void static_route::execute(const http_request& request, http_response& response)
  {
    if (!std::filesystem::exists(location_))
    {
      response.set_status(http_status::not_found);
      response.set_body("Not found");
      return;
    }

    if (!std::filesystem::is_directory(location_))
    {
      response.set_status(http_status::ok);
      response.set_body(read_file(location_));
      return;
    }

    std::filesystem::path file_location = get_file_path(path_,
                                                        request.get_uri(),
                                                        location_);
    if (!std::filesystem::exists(file_location))
    {
      response.set_status(http_status::not_found);
      response.set_body("Not found");
      return;
    }

    response.set_status(http_status::ok);
    response.set_body(read_file(file_location));
  }

  bool static_route::matches(const std::string& path) const
  {
    if (path == path_)
      return true;

    if (path.find("..") != std::string::npos)
      return false;
    if (path.find("//") != std::string::npos)
      return false;
    if (path.find("~") != std::string::npos)
      return false;

    if (std::filesystem::is_directory(location_))
    {
      std::string file_path = path.substr(path_.size());
      if (file_path.starts_with('/'))
      {
        file_path = file_path.erase(0, 1);
      }

      std::filesystem::path file_location = location_ / file_path;
      return std::filesystem::exists(file_location);
    }

    return false;
  }
}