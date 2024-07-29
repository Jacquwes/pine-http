#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include "error.h"
#include "http.h"

namespace pine::http_utils
{
  static constexpr std::string find_body(std::string_view request, size_t& offset, std::error_code& ec)
  {
    size_t start = request.find(crlf, offset);
    if (start == std::string::npos)
    {
      ec = make_error_code(pine::error::parse_error_body);
      return {};
    }

    start += 2;
    offset += request.size() - start;

    return std::string(request.substr(start));
  }

  static std::pair<std::string, std::string> find_header(std::string_view request, size_t& offset, std::error_code& ec)
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

    auto name = std::string(request.substr(start, colon - start));
    auto value = std::string(request.substr(colon + 2, end - colon - 2));
    offset = end;

    return { name, value };
  }

  static std::map<std::string, std::string> find_headers(const std::string& request, size_t& offset, std::error_code& ec)
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

  static http_method find_method(std::string_view request, size_t& offset, std::error_code& ec)
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

  static constexpr std::string find_uri(std::string_view request, size_t& offset, std::error_code& ec)
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

  static constexpr http_version find_version(std::string_view request, size_t& offset, std::error_code& ec)
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