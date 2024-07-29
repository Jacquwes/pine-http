#pragma once

#include <map>
#include <string>
#include <string_view>
#include <system_error>
#include "http.h"

namespace pine
{
  class http_response
  {
  public:
    explicit http_response() = default;

    static http_response parse(const std::string& response, std::error_code& ec);

    constexpr const std::string& get_body() const { return this->body; }
    const std::string& get_header(const std::string& name) const;
    constexpr const std::map<std::string, std::string>& get_headers() const { return this->headers; }
    constexpr http_status get_status() const { return this->status; }
    constexpr http_version get_version() const { return this->version; }

    std::string to_string() const;

    constexpr void set_body(std::string_view value) { this->body = value; }
    void set_header(const std::string& name, const std::string& value) { this->headers.insert_or_assign(name, value); }
    constexpr void set_status(http_status value) { this->status = value; }
    constexpr void set_version(http_version value) { this->version = value; }

  private:
    std::string body;
    std::map<std::string, std::string> headers;
    http_status status = http_status::ok;
    http_version version = http_version::http_1_1;
  };
}