#pragma once

#include <map>
#include <string>

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
}