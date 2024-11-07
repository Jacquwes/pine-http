#pragma once

#include <string>

namespace pine
{
  enum class error_code
  {
    success,
    parse_error_method,
    parse_error_uri,
    parse_error_version,
    parse_error_headers,
    parse_error_body,
    parse_error_status,
    route_not_found,
    method_not_allowed,
    client_not_found,
    connection_closed,
  #ifdef _WIN32
    winsock_error,
    getaddrinfo_error,
  #endif // _WIN32
    coroutine_cancelled,
    path_parameter_conflict,
  };

  class error
  {
  public:
    error(error_code code);
    error(error_code code, const std::string& message);
    error_code code() const;
    const std::string& message() const;

  private:
    error_code code_;
    std::string message_;
  };
}