#include <cstring>
#include <format>
#include <map>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include "error.h"
#include "expected.h"
#include "http.h"

namespace pine::http_utils
{
  std::expected<std::string_view, pine::error>
    try_get_body(std::string_view request, size_t& offset)
  {
    if (offset >= request.size())
    {
      return std::make_unexpected(
        error(error_code::parse_error_body,
              "There is no body at this position."));
    }

    std::string_view body = request.substr(offset);
    offset = request.size();

    return body;
  }

  std::expected<std::pair<std::string, std::string_view>, pine::error>
    try_get_header(std::string_view request, size_t& offset)
  {
    size_t start = offset;
    size_t end = request.find(crlf, start);
    size_t colon = request.find(':', start);
    if (colon == std::string::npos || end == std::string::npos || colon > end)
    {
      return std::make_unexpected(
        error(error_code::parse_error_headers,
              "The header is not formatted correctly."));

    }

    size_t name_start = start;
    size_t name_end = colon;
    size_t value_start = colon + 2;
    size_t value_end = end;

    offset = end + strlen(crlf);

    const auto& name = std::string(
      request.substr(name_start, name_end - name_start));
    const auto& value = std::string(
      request.substr(value_start, value_end - value_start));

    return std::make_pair(name, value);
  }

  std::expected<std::unordered_map<std::string, std::string>, pine::error>
    try_get_headers(std::string_view request, size_t& offset)
  {
    std::unordered_map<std::string, std::string> result;

    while (true)
    {
      if (bool line_exists = offset < request.size(); !line_exists)
        return result;
      if (bool line_is_empty = request.substr(offset).starts_with(crlf))
      {
        offset += strlen(crlf);
        return result;
      }

      const auto& header_result = try_get_header(request, offset);
      if (!header_result)
        return std::make_unexpected(header_result.error());
      const auto& [name, value] = header_result.value();

      result.insert_or_assign(name, value);
    }

    return {};
  }

  std::expected<http_method, pine::error>
    try_get_method(std::string_view request, size_t& offset)
  {
    for (const auto& [method, method_string] : http_method_strings)
    {
      if (request.substr(offset).starts_with(method_string))
      {
        offset += method_string.size();
        return method;
      }
    }

    return std::make_unexpected(error(error_code::parse_error_method,
                                      "The method is not recognized."));
  }

  std::expected<http_status, pine::error>
    try_get_status(std::string_view request, size_t& offset)
  {
    for (const auto& [status, status_string] : http_status_strings)
    {
      const auto& status_code_string =
        std::to_string(static_cast<int>(status));
      const std::string& expected_status =
        std::format("{} {}", status_code_string, status_string);

      if (request.substr(offset).starts_with(expected_status))
      {
        offset += expected_status.size();
        return status;
      }
    }

    return std::make_unexpected(error(error_code::parse_error_status,
                                      "The status is not recognized."));
  }

  std::expected<std::string_view, pine::error>
    try_get_uri(std::string_view request, size_t& offset)
  {
    if (request.at(offset) != '/')
    {
      return std::make_unexpected(
        error(error_code::parse_error_uri,
              "No URI was found in the request."));
    }

    size_t start = offset;
    size_t end = request.find(' ', start);
    if (end == std::string::npos)
    {
      return std::make_unexpected(error(
        error_code::parse_error_uri,
        "The URI is not formatted correctly."));
    }

    offset = end;
    return std::string(request.substr(start, end - start));
  }

  std::expected<http_version, pine::error>
    try_get_version(std::string_view request,
                    size_t& offset)
  {
    for (const auto& [version, version_string] : http_version_strings)
    {
      if (request.substr(offset).starts_with(version_string))
      {
        offset += version_string.size();
        return version;
      }
    }

    return std::make_unexpected(error(error_code::parse_error_version,
                                      "The version is not recognized."));
  }
}
