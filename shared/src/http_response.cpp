#include "error.h"
#include <format>
#include "http.h"
#include "http_response.h"

namespace pine
{
  http_response http_response::parse(const std::string& response, std::error_code& ec)
  {
    http_response result{};

    size_t offset = 0;

    result.version = http_utils::find_version(response, offset, ec);
    if (ec) return result;

    result.status = http_utils::find_status(response, offset, ec);
    if (ec) return result;

    result.headers = http_utils::find_headers(response, offset, ec);
    if (ec) return result;

    result.body = http_utils::find_body(response, offset, ec);

    return result;
  }

  const std::string& http_response::get_header(const std::string& name) const
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
    std::string version_string = http_version_strings.at(this->version);
    std::string status_code_string = std::to_string(static_cast<int>(this->status));
    std::string status_message_string = http_status_strings.at(this->status);

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