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

    std::string file_path = request.get_uri().substr(path_.size());
    if (file_path.size() == 0)
    {
      file_path += "index.html";
    }

    if (file_path.starts_with('/'))
    {
      file_path = file_path.erase(0, 1);
    }

    std::filesystem::path file_location = location_ / file_path;
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