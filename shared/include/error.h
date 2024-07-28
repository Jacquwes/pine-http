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
  };

  class error_category : public std::error_category
  {
  public:
    inline const char* name() const noexcept override { return "pine"; }

    inline std::string message(int code) const override
    {
      switch (static_cast<error>(code))
      {
      case error::success:
        return "Success";
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