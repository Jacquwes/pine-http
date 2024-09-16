#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include "expected.h"
#include "http.h"

namespace pine
{
  /// @brief Represents an HTTP request.
  class http_request
  {
  public:
    /// @brief Default constructor.
    explicit http_request() = default;

    /// @brief Constructor that initializes the HTTP request with the
    /// given parameters.
    /// @param method The HTTP method of the request.
    /// @param uri The URI of the request.
    /// @param version The HTTP version of the request.
    /// @param headers The headers of the request.
    /// @param body The body of the request.
    explicit http_request(pine::http_method method,
                          const std::string& uri,
                          pine::http_version version,
                          const std::map<std::string, std::string>& headers,
                          const std::string& body);

    /// @brief Parses an HTTP request from a string.
    /// @param request The string representation of the request.
    /// @return An expected object containing the parsed HTTP request or an
    /// error code.
    static std::expected<http_request, std::error_code>
      parse(const std::string& request);

    /// @brief Gets the body of the HTTP request.
    /// @return The body of the request.
    constexpr const std::string& get_body() const
    {
      return this->body;
    }

    /// @brief Gets the value of the specified header from the HTTP request.
    /// @param name The name of the header.
    /// @return The value of the header.
    const std::string& get_header(const std::string& name) const;

    /// @brief Gets the headers of the HTTP request.
    /// @return The headers of the request.
    constexpr const std::map<std::string, std::string>& get_headers() const
    {
      return this->headers;
    }

    /// @brief Gets the HTTP method of the request.
    /// @return The HTTP method.
    constexpr pine::http_method get_method() const
    {
      return this->method;
    }

    /// @brief Gets the URI of the request.
    /// @return The URI.
    constexpr const std::string& get_uri() const
    {
      return this->uri;
    }

    /// @brief Gets the HTTP version of the request.
    /// @return The HTTP version.
    constexpr pine::http_version get_version() const
    {
      return this->version;
    }

    /// @brief Converts the HTTP request to a string representation.
    /// @return The string representation of the request.
    std::string to_string() const;

    /// @brief Sets the body of the HTTP request.
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

    /// @brief Sets the value of the specified header in the HTTP request.
    /// @param name The name of the header.
    /// @param value The value of the header.
    void set_header(const std::string& name, const std::string& value)
    {
      this->headers.insert_or_assign(name, value);
    }

    /// @brief Sets the HTTP method of the request.
    /// @param value The new HTTP method.
    constexpr void set_method(pine::http_method value)
    {
      this->method = value;
    }

    /// @brief Sets the URI of the request.
    /// @param value The new URI.
    constexpr void set_uri(std::string_view value)
    {
      this->uri = value;
    }

    /// @brief Sets the HTTP version of the request.
    /// @param value The new HTTP version.
    constexpr void set_version(pine::http_version value)
    {
      this->version = value;
    }

  private:
    pine::http_method method = pine::http_method::get;
    std::string uri;
    pine::http_version version = pine::http_version::http_1_1;
    std::map<std::string, std::string> headers;
    std::string body;
  };
}