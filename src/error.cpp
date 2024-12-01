#include <pine/error.h>
#include <string>

namespace pine
{
  error::error(error_code code)
    : code_(code)
  {}

  error::error(error_code code, const std::string& message)
    : code_(code)
    , message_(message)
  {}

  error_code error::code() const
  {
    return code_;
  }

  const std::string& error::message() const
  {
    return message_;
  }
}