#include <cstring>
#include <map>
#include <string>
#include "error.h"
#include "expected.h"
#include "http.h"
#include "http_request.h"

namespace pine
{
  http_request::http_request(pine::http_method method,
                             std::string_view uri,
                             pine::http_version version,
                             const std::unordered_map<std::string, std::string_view>& headers,
                             std::string_view body)
    : method(method), uri(uri), version(version), headers(headers), body(body)
  {}

  std::expected<http_request, pine::error>
    http_request::parse(std::string_view request)
  {
    http_request result;

    size_t offset = 0;
    const auto& method_result = http_utils::try_get_method(request, offset);
    if (!method_result)
      return std::make_unexpected(method_result.error());
    result.method = method_result.value();
    offset += strlen(" ");

    const auto& uri_result = http_utils::try_get_uri(request, offset);
    if (!uri_result)
      return std::make_unexpected(uri_result.error());
    result.uri = uri_result.value();
    offset += strlen(" ");

    const auto& version_result = http_utils::try_get_version(request, offset);
    if (!version_result)
      return std::make_unexpected(version_result.error());
    result.version = version_result.value();
    offset += strlen(crlf);

    const auto& headers_result = http_utils::try_get_headers(request, offset);
    if (!headers_result)
      return std::make_unexpected(headers_result.error());
    result.headers = headers_result.value();

    if (offset < request.size())
    {
      const auto& body_result = http_utils::try_get_body(request, offset);
      if (!body_result)
        return std::make_unexpected(body_result.error());
      result.body = body_result.value();
    }

    return result;
  }

  std::string_view http_request::get_header(const std::string& name) const
  {
    if (this->headers.contains(name))
    {
      return this->headers.at(name);
    }
    else
    {
      static const std::string empty;
      return empty;
    }
  }

  std::string http_request::to_string() const
  {
    std::string result;

    result += pine::http_method_strings.at(this->method);
    result += " ";
    result += this->uri + " ";
    result += pine::http_version_strings.at(this->version);
    result += crlf;

    for (const auto& [name, value] : this->headers)
    {
      result += name + ": " + std::string(value) + crlf;
    }

    result += crlf;
    result += this->body;

    return result;
  }
}
