#include <cstring>
#include <expected.h>
#include <format>
#include <string>
#include "error.h"
#include "http.h"
#include "http_response.h"

namespace pine
{
  std::expected<http_response, pine::error>
    http_response::parse(const std::string& response)
  {
    http_response result{};

    size_t offset = 0;

    const auto& version_result = http_utils::try_get_version(response, offset);
    if (!version_result)
      return std::make_unexpected(version_result.error());
    result.version = version_result.value();
    offset += strlen(" ");

    const auto& status_result = http_utils::try_get_status(response, offset);
    if (!status_result)
      return std::make_unexpected(status_result.error());
    result.status = status_result.value();
    offset += strlen(crlf);

    const auto& headers_result = http_utils::try_get_headers(response, offset);
    if (!headers_result)
      return std::make_unexpected(headers_result.error());
    result.headers = headers_result.value();

    if (offset < response.size())
    {
      const auto& body_result = http_utils::try_get_body(response, offset);
      if (!body_result)
        return std::make_unexpected(body_result.error());
      result.body = body_result.value();
    }

    return result;
  }

  const std::string&
    http_response::get_header(const std::string& name) const
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

  std::string http_response::to_string() const
  {
    std::string result;
    const std::string& version_string = http_version_strings.at(this->version);
    const std::string& status_code_string = std::to_string(static_cast<int>(this->status));
    const std::string& status_message_string = http_status_strings.at(this->status);

    result += std::format("{} {} {}",
                          version_string,
                          status_code_string,
                          status_message_string);

    result += crlf;

    for (const auto& [name, value] : this->headers)
    {
      result += name + ": " + value + crlf;
    }

    result += crlf;
    result += this->body;

    return result;
  }
}