#pragma once

#include <map>
#include <string>
#include <string_view>
#include "error.h"
#include "expected.h"

namespace pine
{
  /// @brief Represents an HTTP method.
  /// More information at https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods.
  enum class http_method
  {
    /// The GET method requests a representation of the specified resource. 
    /// Requests using GET should only retrieve data and should not contain a
    /// request content.
    get = 1,
    /// The HEAD method asks for a response identical to that of a GET request,
    /// but without the response body.
    head,
    /// The POST method is used to submit an entity to the specified resource, 
    /// often causing a change in state or side effects on the server.
    post,
    /// The PUT method replaces all current representations of the target 
    /// resource with the request payload.
    put,
    /// The DELETE method deletes the specified resource.
    delete_,
    /// The CONNECT method establishes a tunnel to the server identified by the
    /// target resource.
    connect,
    /// The OPTIONS method is used to describe the communication options for 
    /// the target resource.
    options,
    /// The TRACE method performs a message loop-back test along the path to
    /// the target resource.
    trace,
    /// The PATCH method is used to apply partial modifications to a resource.
    patch,
  };

  constexpr auto http_method_count = 9;

  /// @brief Map of HTTP methods to their string representations.
  inline const std::map<http_method, std::string> http_method_strings
  {
    { http_method::get, "GET" },
    { http_method::head, "HEAD" },
    { http_method::post, "POST" },
    { http_method::put, "PUT" },
    { http_method::delete_, "DELETE" },
    { http_method::connect, "CONNECT" },
    { http_method::options, "OPTIONS" },
    { http_method::trace, "TRACE" },
    { http_method::patch, "PATCH" },
  };

  /// @brief Represents an HTTP status code.
  enum class http_status
  {
    ok = 200, /// The OK status code.
    bad_request = 400, /// The Bad Request status code.
    not_found = 404, /// The Not Found status code.
    method_not_allowed = 405, /// The Method Not Allowed status code.
    internal_server_error = 500, /// The Internal Server Error status code.
  };

  /// @brief Map of HTTP status codes to their string representations.
  inline const std::map<http_status, std::string> http_status_strings
  {
    { http_status::ok, "OK" },
    { http_status::bad_request, "Bad Request" },
    { http_status::not_found, "Not Found" },
    { http_status::method_not_allowed, "Method Not Allowed" },
    { http_status::internal_server_error, "Internal Server Error" },
  };

  /// @brief Represents an HTTP version.
  enum class http_version
  {
    http_1_1, /// HTTP version 1.1.
  };

  /// @brief Map of HTTP versions to their string representations.
  inline const std::map<http_version, std::string> http_version_strings
  {
    { http_version::http_1_1, "HTTP/1.1" },
  };

  /// @brief Carriage return and line feed sequence.
  static constexpr const char* crlf = "\r\n";

  namespace http_utils
  {
    /// @brief Tries to extract the body from an HTTP request.
    /// @param request The HTTP request.
    /// @param offset The offset in the request where the body starts.
    /// @return The extracted body as a string.
    std::expected<std::string, pine::error>
      try_get_body(std::string_view request, size_t& offset);

    /// @brief Tries to extract the headers from an HTTP request.
    /// @param request The HTTP request.
    /// @param offset The offset in the request where the headers start.
    /// @return The extracted headers as a map of key-value pairs.
    std::expected<std::map<std::string, std::string>, pine::error>
      try_get_headers(const std::string& request, size_t& offset);

    /// @brief Tries to extract a single header from an HTTP request.
    /// @param request The HTTP request.
    /// @param offset The offset in the request where the header starts.
    /// @return The extracted header as a pair of key and value.
    std::expected<std::pair<std::string, std::string>, pine::error>
      try_get_header(std::string_view request, size_t& offset);

    /// @brief Tries to extract the HTTP method from an HTTP request.
    /// @param request The HTTP request.
    /// @param offset The offset in the request where the method starts.
    /// @return The extracted HTTP method.
    std::expected<http_method, pine::error>
      try_get_method(std::string_view request, size_t& offset);

    /// @brief Tries to extract the HTTP status from an HTTP response.
    /// @param response The HTTP response.
    /// @param offset The offset in the response where the status starts.
    /// @return The extracted HTTP status.
    std::expected<http_status, pine::error>
      try_get_status(std::string_view response, size_t& offset);

    /// @brief Tries to extract the URI from an HTTP request.
    /// @param request The HTTP request.
    /// @param offset The offset in the request where the URI starts.
    /// @return The extracted URI as a string.
    std::expected<std::string, pine::error>
      try_get_uri(std::string_view request, size_t& offset);

    /// @brief Tries to extract the HTTP version from an HTTP request.
    /// @param request The HTTP request.
    /// @param offset The offset in the request where the version starts.
    /// @return The extracted HTTP version.
    std::expected<http_version, pine::error>
      try_get_version(std::string_view request, size_t& offset);
  }
}
