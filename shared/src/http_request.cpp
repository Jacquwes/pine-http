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
    result.method = find_method(request, offset, ec);
    if (ec) return result;

    result.uri = find_uri(request, offset, ec);
    if (ec) return result;

    result.version = find_version(request, offset, ec);
    if (ec) return result;

    result.headers = find_headers(request, offset, ec);
    if (ec) return result;

    result.body = find_body(request, offset, ec);
    if (ec) return result;

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

  constexpr std::string http_request::find_body(const std::string& request, size_t& offset, std::error_code& ec)
  {
    size_t start = request.find(crlf, offset);
    if (start == std::string::npos)
    {
      ec = make_error_code(pine::error::parse_error_body);
      return {};
    }

    start += 2;
    return request.substr(start);
  }

  std::pair<std::string, std::string> http_request::find_header(const std::string& request, size_t& offset, std::error_code& ec)
  {
    size_t start = request.find(crlf, offset);
    if (start == std::string::npos)
    {
      ec = make_error_code(error::parse_error_headers);
      return {};
    }

    start += strlen(crlf);

    if (bool line_is_empty = start == request.find(crlf, start);
        line_is_empty)
    {
      offset += strlen(crlf);
      return { {}, {} };
    }

    size_t colon = request.find(':', offset);
    if (colon == std::string::npos || colon <= start)
    {
      ec = make_error_code(error::parse_error_headers);
      return {};
    }

    size_t end = request.find(crlf, colon);
    if (end == std::string::npos)
    {
      ec = make_error_code(error::parse_error_headers);
      return {};
    }

    std::string name = request.substr(start, colon - start);
    std::string value = request.substr(colon + 2, end - colon - 2);
    offset = end;

    return { name, value };
  }

  std::map<std::string, std::string> http_request::find_headers(const std::string& request, size_t& offset, std::error_code& ec)
  {
    std::map<std::string, std::string> result;

    while (true)
    {
      auto [name, value] = find_header(request, offset, ec);
      if (ec) return {};

      if (name.empty())
      {
        break;
      }

      result.insert_or_assign(name, value);
    }

    return result;
  }

  pine::http_method http_request::find_method(std::string_view request, size_t& offset, std::error_code& ec)
  {
    using enum http_method;

    if (std::string_view get_string = http_method_strings.at(get);
        request.starts_with(get_string))
    {
      offset += get_string.size();
      return get;
    }
    else if (std::string_view head_string = http_method_strings.at(head);
             request.starts_with(head_string))
    {
      offset += head_string.size();
      return head;
    }

    ec = make_error_code(error::parse_error_method);
    return {};
  }

  constexpr std::string http_request::find_uri(std::string_view request, size_t& offset, std::error_code& ec)
  {
    size_t start = request.find('/', offset);
    if (start == std::string::npos)
    {
      ec = make_error_code(error::parse_error_uri);
      return {};
    }

    size_t end = request.find(' ', start);
    if (end == std::string::npos)
    {
      ec = make_error_code(error::parse_error_uri);
      return {};
    }

    offset = end;
    auto uri = std::string(request.substr(start, end - start));
    return uri;
  }

  constexpr pine::http_version http_request::find_version(std::string_view request, size_t& offset, std::error_code& ec)
  {
    size_t start = request.find(' ', offset);
    if (start == std::string::npos)
    {
      ec = make_error_code(error::parse_error_version);
      return {};
    }

    size_t eol = request.find(crlf, start);
    if (eol == std::string::npos)
    {
      ec = make_error_code(error::parse_error_version);
      return {};
    }

    std::string_view http_1_1_string = http_version_strings.at(http_version::http_1_1);
    if (size_t version_index = request.find(http_1_1_string, start);
        version_index != std::string::npos &&
        version_index < eol)
    {
      offset += http_1_1_string.size();
      return http_version::http_1_1;
    }

    ec = make_error_code(error::parse_error_version);
    return {};
  }
}
