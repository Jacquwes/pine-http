#pragma once

#include <string>
#include <system_error>

namespace pine
{
  enum class error
  {
    success,
    parse_error_method,
    parse_error_uri,
    parse_error_version,
    parse_error_headers,
    parse_error_body,
    parse_error_status,
  };

  class error_category : public std::error_category
  {
  public:
    inline const char* name() const noexcept override { return "pine"; }

    inline std::string message(int code) const override
    {
      using enum error;

      switch (static_cast<error>(code))
      {
      case success:
        return "Success";
      case parse_error_method:
        return "Parse error: method";
      case parse_error_uri:
        return "Parse error: URI";
      case parse_error_version:
        return "Parse error: version";
      case parse_error_headers:
        return "Parse error: headers";
      case parse_error_body:
        return "Parse error: body";
      case parse_error_status:
        return "Parse error: status";
      default:
        return "Unknown";
      }
    }
  };

  inline const error_category global_error_category;

  inline std::error_code make_error_code(error e) noexcept
  {
    return std::error_code(static_cast<int>(e), global_error_category);
  }
}