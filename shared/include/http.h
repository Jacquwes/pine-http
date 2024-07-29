#pragma once

#include <map>
#include <string>
#include <string_view>
#include <system_error>

namespace pine
{
  enum class http_method
  {
    get,
    head,
    post,
  };

  inline const std::map<http_method, std::string> http_method_strings
  {
    { http_method::get, "GET" },
    { http_method::head, "HEAD" },
    { http_method::post, "POST" },
  };

  enum class http_status
  {
    ok = 200,
    bad_request = 400,
    not_found = 404,
    internal_server_error = 500,
  };

  inline const std::map<http_status, std::string> http_status_strings
  {
    { http_status::ok, "OK" },
    { http_status::bad_request, "Bad Request" },
    { http_status::not_found, "Not Found" },
    { http_status::internal_server_error, "Internal Server Error" },
  };

  enum class http_version
  {
    http_1_1,
  };

  inline const std::map<http_version, std::string> http_version_strings
  {
    { http_version::http_1_1, "HTTP/1.1" },
  };

  static constexpr const char* crlf = "\r\n";

  namespace http_utils
  {
    std::string find_body(std::string_view request, size_t& offset, std::error_code& ec);
    std::map<std::string, std::string> find_headers(const std::string& request, size_t& offset, std::error_code& ec);
    std::pair<std::string, std::string> find_header(std::string_view request, size_t& offset, std::error_code& ec);
    http_method find_method(std::string_view request, size_t& offset, std::error_code& ec);
    http_status find_status(std::string_view response, size_t& offset, std::error_code& ec);
    std::string find_uri(std::string_view request, size_t& offset, std::error_code& ec);
    http_version find_version(std::string_view request, size_t& offset, std::error_code& ec);
  }
}
