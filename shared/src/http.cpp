#include <cstring>
#include <format>
#include <map>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include "error.h"
#include "http.h"

namespace pine::http_utils
{
  std::string try_get_body(std::string_view request,
                           size_t& offset,
                           std::error_code& ec)
  {
    if (offset >= request.size())
    {
      ec = make_error_code(error::parse_error_body);
      return {};
    }

    auto body = std::string(request.substr(offset));
    offset = request.size();

    return body;
  }

  std::pair<std::string, std::string> try_get_header(std::string_view request,
                                                     size_t& offset,
                                                     std::error_code& ec)
  {
    size_t start = offset;
    size_t end = request.find(crlf, start);
    size_t colon = request.find(':', start);
    if (colon == std::string::npos || end == std::string::npos || colon > end)
    {
      ec = make_error_code(error::parse_error_headers);
      return {};
    }

    size_t name_start = start;
    size_t name_end = colon;
    size_t value_start = colon + 2;
    size_t value_end = end;

    offset = end + strlen(crlf);

    auto name = std::string(request.substr(name_start, name_end - name_start));
    auto value = std::string(request.substr(value_start, value_end - value_start));

    return { name, value };
  }

  std::map<std::string, std::string> try_get_headers(const std::string& request,
                                                     size_t& offset,
                                                     std::error_code& ec)
  {
    std::map<std::string, std::string> result;

    while (true)
    {
      if (bool line_exists = offset < request.size(); !line_exists)
        return result;
      if (bool line_is_empty = request.substr(offset).starts_with(crlf))
      {
        offset += strlen(crlf);
        return result;
      }

      auto [name, value] = try_get_header(request, offset, ec);
      if (ec) break;

      result.insert_or_assign(name, value);
    }

    return {};
  }

  http_method try_get_method(std::string_view request,
                             size_t& offset,
                             std::error_code& ec)
  {
    for (const auto& [method, method_string] : http_method_strings)
    {
      if (request.substr(offset).starts_with(method_string))
      {
        offset += method_string.size();
        return method;
      }
    }

    ec = make_error_code(error::parse_error_method);
    return {};
  }

  http_status try_get_status(std::string_view request,
                             size_t& offset,
                             std::error_code& ec)
  {
    for (const auto& [status, status_string] : http_status_strings)
    {
      auto status_code_string = std::to_string(static_cast<int>(status));
      std::string expected_status = std::format("{} {}", status_code_string, status_string);

      if (request.substr(offset).starts_with(expected_status))
      {
        offset += expected_status.size();
        return status;
      }
    }

    ec = make_error_code(error::parse_error_status);
    return {};
  }

  std::string try_get_uri(std::string_view request,
                          size_t& offset,
                          std::error_code& ec)
  {
    if (request.at(offset) != '/')
    {
      ec = make_error_code(error::parse_error_uri);
      return {};
    }

    size_t start = offset;
    size_t end = request.find(' ', start);
    if (end == std::string::npos)
    {
      ec = make_error_code(error::parse_error_uri);
      return {};
    }

    offset = end;
    return std::string(request.substr(start, end - start));
  }

  http_version try_get_version(std::string_view request,
                               size_t& offset,
                               std::error_code& ec)
  {
    for (const auto& [version, version_string] : http_version_strings)
    {
      if (request.substr(offset).starts_with(version_string))
      {
        offset += version_string.size();
        return version;
      }
    }

    ec = make_error_code(error::parse_error_version);
    return {};
  }
}
