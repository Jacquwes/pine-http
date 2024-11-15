#pragma once

#include <map>
#include <string>
#include <string_view>
#include "error.h"
#include "expected.h"
#include "http.h"

namespace pine
{
  /// @brief Represents an HTTP response.
  class http_response
  {
  public:
    /// @brief Default constructor.
    explicit http_response() = default;

    /// @brief Parses an HTTP response from a string.
    /// @param response The string representation of the HTTP response.
    /// @return An expected object containing the parsed HTTP response or an error code.
    static std::expected<http_response, pine::error>
      parse(std::string_view response);

    /// @brief Gets the body of the HTTP response.
    /// @return A constant reference to the body string.
    constexpr std::string_view get_body() const
    {
      return this->body;
    }

    /// @brief Gets the value of a specific header in the HTTP response.
    /// @param name The name of the header.
    /// @return A constant reference to the header value.
    std::string_view get_header(const std::string& name) const;

    /// @brief Gets all the headers in the HTTP response.
    /// @return A constant reference to the headers map.
    constexpr const std::map<std::string, std::string_view>& get_headers() const
    {
      return this->headers;
    }

    /// @brief Gets the status code of the HTTP response.
    /// @return The HTTP status code.
    constexpr http_status get_status() const
    {
      return this->status;
    }

    /// @brief Gets the version of the HTTP response.
    /// @return The HTTP version.
    constexpr http_version get_version() const
    {
      return this->version;
    }

    /// @brief Converts the HTTP response to a string representation.
    /// @return The string representation of the HTTP response.
    std::string to_string() const;

    /// @brief Sets the body of the HTTP response.
    /// @param value The new body value.
    void set_body(std::string_view value)
    {
      this->body = value;
      if (value.empty())
      {
        this->headers.erase("Content-Length");
      }
      else
      {
        this->headers.insert_or_assign("Content-Length", std::to_string(value.size()));
      }
    }

    /// @brief Sets a header in the HTTP response.
    /// @param name The name of the header.
    /// @param value The value of the header.
    void set_header(const std::string& name, std::string_view value)
    {
      this->headers.insert_or_assign(name, value);
    }

    /// @brief Sets the status code of the HTTP response.
    /// @param value The new status code.
    constexpr void set_status(http_status value)
    {
      this->status = value;
    }

    /// @brief Sets the version of the HTTP response.
    /// @param value The new version.
    constexpr void set_version(http_version value)
    {
      this->version = value;
    }

  private:
    std::string body;
    std::map<std::string, std::string_view> headers;
    http_status status = http_status::ok;
    http_version version = http_version::http_1_1;
  };
}