#pragma once

#include <http_request.h>
#include <http_response.h>
#include <route_base.h>
#include <string>
#include <filesystem>

namespace pine
{
  class static_route : public route_base
  {
  public:
    static_route() = delete;

    /// @brief Create a new static route. This route will serve files from the
    /// specified location. 
    /// 
    /// - If the localtion does not exist, the route will
    /// respond with a 404 status code.\n
    /// 
    /// - If the location is a directory, the
    /// route will attempt to serve the file "index.html" from that directory,
    /// or the corresponding file for the requested path.
    /// 
    /// - If the location is a file, the route will serve that file.
    /// @param path The path to match in order to serve files from the location.
    /// @param location The location to serve files from.
    static_route(std::string&& path, std::filesystem::path&& location)
      : path_(std::move(path))
      , location_(std::move(location))
    {}

    ~static_route() = default;

    const std::string& path() const override { return path_; }

    /// @brief Execute the route. This function will attempt to serve the file
    /// from the location specified when the route was created and the path of
    /// the request.
    /// 
    /// This function assumes that the request path matches the route path.
    /// @param request Request received from the client.
    /// @param response Response to send to the client.
    void execute(const http_request& request, http_response& response) override;

    /// @brief Check if the route matches the given path. 
    /// 
    /// - If the route location
    /// is a directory, this function checks if the index.html file exists in
    /// that directory. 
    /// 
    /// - If the route location is a file, this function checks if
    /// the route location matches the path.
    /// @param path Path to check.
    /// @return True if the route matches the path, false otherwise.
    bool matches(const std::string& path) const override;

  private:
    std::string path_;
    std::filesystem::path location_;
  };
}