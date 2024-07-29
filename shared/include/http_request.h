#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include "http.h"

namespace pine
{
  class http_request
  {
  public:
    explicit http_request() = default;
    explicit http_request(pine::http_method method,
                          const std::string& uri,
                          pine::http_version version,
                          const std::map<std::string, std::string>& headers,
                          const std::string& body);

    static http_request parse(const std::string& request, std::error_code& ec);

    constexpr const std::string& get_body() const { return this->body; }
    const std::string& get_header(const std::string& name) const;
    constexpr const std::map<std::string, std::string>& get_headers() const { return this->headers; }
    constexpr pine::http_method get_method() const { return this->method; }
    constexpr const std::string& get_uri() const { return this->uri; }
    constexpr pine::http_version get_version() const { return this->version; }

    std::string to_string() const;

    constexpr void set_body(std::string_view value) { this->body = value; }
    void set_header(const std::string& name, const std::string& value) { this->headers.insert_or_assign(name, value); }
    constexpr void set_method(pine::http_method value) { this->method = value; }
    constexpr void set_uri(std::string_view value) { this->uri = value; }
    constexpr void set_version(pine::http_version value) { this->version = value; }

  private:
    pine::http_method method = pine::http_method::get;
    std::string uri;
    pine::http_version version = pine::http_version::http_1_1;
    std::map<std::string, std::string> headers;
    std::string body;

    static constexpr const char* crlf = "\r\n";

    static constexpr std::string find_body(const std::string& request, size_t& offset, std::error_code& ec);
    static std::map<std::string, std::string> find_headers(const std::string& request, size_t& offset, std::error_code& ec);
    static std::pair<std::string, std::string> find_header(const std::string& request, size_t& offset, std::error_code& ec);
    static pine::http_method find_method(std::string_view request, size_t& offset, std::error_code& ec);
    static constexpr std::string find_uri(std::string_view request, size_t& offset, std::error_code& ec);
    static constexpr pine::http_version find_version(std::string_view request, size_t& offset, std::error_code& ec);
  };
}