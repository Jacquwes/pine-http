#include <cstring>
#include <map>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include "error.h"
#include "http.h"
#include "http_request.h"

namespace pine
{
  http_request::http_request(pine::http_method method,
                             const std::string& uri,
                             pine::http_version version,
                             const std::map<std::string, std::string>& headers,
                             const std::string& body)
    : method(method), uri(uri), version(version), headers(headers), body(body)
  {}

  http_request http_request::parse(const std::string& request, std::error_code& ec)
  {
    http_request result;

    size_t offset = 0;
    result.method = http_utils::try_get_method(request, offset, ec);
    if (ec) return result;
    offset += strlen(" ");

    result.uri = http_utils::try_get_uri(request, offset, ec);
    if (ec) return result;
    offset += strlen(" ");

    result.version = http_utils::try_get_version(request, offset, ec);
    if (ec) return result;
    offset += strlen(crlf);

    result.headers = http_utils::try_get_headers(request, offset, ec);
    if (ec) return result;

    if (offset < request.size())
    {
      result.body = http_utils::try_get_body(request, offset, ec);
    }

    return result;
  }

  const std::string& http_request::get_header(const std::string& name) const
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

    result += pine::http_method_strings.at(this->method) + " ";
    result += this->uri + " ";
    result += pine::http_version_strings.at(this->version) + crlf;

    for (const auto& [name, value] : this->headers)
    {
      result += name + ": " + value + crlf;
    }

    result += crlf;
    result += this->body;

    return result;
  }
}
