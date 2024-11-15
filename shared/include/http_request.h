#pragma once

#include <charconv>
#include <error.h>
#include <expected.h>
#include <http.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

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
                          std::string_view _viewuri,
                          pine::http_version version,
                          const std::map<std::string, std::string_view>& headers,
                          std::string_view _viewbody);

    /// @brief Parses an HTTP request from a string.
    /// @param request The string representation of the request.
    /// @return An expected object containing the parsed HTTP request or an
    /// error code.
    static std::expected<http_request, pine::error>
      parse(std::string_view request);

    /// @brief Adds a path parameter to the HTTP request.
    /// @param name The name of the path parameter.
    /// @param value The value of the path parameter.
    void add_path_param(std::string_view name, std::string_view value)
    {
      this->path_params.insert_or_assign(std::string(name), value);
    }

    /// @brief Gets the body of the HTTP request.
    /// @return The body of the request.
    constexpr std::string_view get_body() const
    {
      return this->body;
    }

    /// @brief Gets the value of the specified header from the HTTP request.
    /// @param name The name of the header.
    /// @return The value of the header.
    std::string_view get_header(const std::string& name) const;

    /// @brief Gets the headers of the HTTP request.
    /// @return The headers of the request.
    constexpr const std::map<std::string, std::string_view>& get_headers() const
    {
      return this->headers;
    }

    /// @brief Gets the HTTP method of the request.
    /// @return The HTTP method.
    constexpr pine::http_method get_method() const
    {
      return this->method;
    }

    /// @brief Gets the value of the specified path parameter from the URI.
    /// @tparam T The type of the path parameter.
    /// @param name The name of the path parameter.
    /// @return The value of the path parameter.
    template <typename T>
      requires std::is_integral_v<T> ||
    std::is_floating_point_v<T> ||
      std::is_same_v<T, std::string> ||
      std::is_same_v<T, std::string_view>
      std::expected<T, error> get_path_param(std::string_view name) const
    {
      auto it = this->path_params.find(std::string(name));
      if (it == this->path_params.end())
        return std::make_unexpected(
          error(error_code::parameter_not_found,
                "Parameter not found: " + std::string(name)));

      if constexpr (std::is_same_v<T, std::string>)
        return std::string(it->second);
      else if constexpr (std::is_same_v<T, std::string_view>)
        return it->second;
      else
      {
        T value;
        if (std::from_chars(it->second.data(),
                            it->second.data() + it->second.size(),
                            value).ec != std::errc{})
          return std::make_unexpected(
            error(error_code::invalid_parameter,
                  "Invalid parameter: " + std::string(name)));

        return value;
      }
    }


    /// @brief Gets the URI of the request.
    /// @return The URI.
    constexpr std::string_view get_uri() const
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
    void set_header(std::string_view name, std::string_view value)
    {
      this->headers.insert_or_assign(std::string(name), value);
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
    std::map<std::string, std::string_view> headers;
    std::string body;
    std::unordered_map<std::string, std::string_view> path_params;
  };
}