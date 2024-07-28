#include <gtest/gtest.h>
#include <map>
#include <string>
#include <system_error>
#include <type_traits>
#include "error.h"
#include "http.h"
#include "http_request.h"

TEST(HttpRequestTest, ParseValidRequest)
{
  std::string request = "GET /api/users HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "User-Agent: Mozilla/5.0\r\n"
    "Content-Length: 10\r\n"
    "\r\n"
    "Hello World";

  std::error_code ec;
  pine::http_request parsedRequest = pine::http_request::parse(request, ec);

  ASSERT_FALSE(ec);
  ASSERT_EQ(pine::http_method::get, parsedRequest.get_method());
  ASSERT_EQ("/api/users", parsedRequest.get_uri());
  ASSERT_EQ(pine::http_version::http_1_1, parsedRequest.get_version());

  std::map<std::string, std::string> expectedHeaders = {
    {"Host", "example.com"},
    {"User-Agent", "Mozilla/5.0"},
    {"Content-Length", "10"}
  };
  ASSERT_EQ(expectedHeaders, parsedRequest.get_headers());

  ASSERT_EQ("Hello World", parsedRequest.get_body());
}

TEST(HttpRequestTest, ParseInvalidRequest)
{
  std::string request = "INVALID REQUEST";

  std::error_code ec;
  pine::http_request parsedRequest = pine::http_request::parse(request, ec);

  ASSERT_TRUE(ec);
  ASSERT_EQ(static_cast<int>(pine::error::parse_error_method), ec.value());
  ASSERT_EQ(pine::http_method{}, parsedRequest.get_method());
  ASSERT_TRUE(parsedRequest.get_uri().empty());
  ASSERT_EQ(pine::http_version{}, parsedRequest.get_version());
  ASSERT_TRUE(parsedRequest.get_headers().empty());
  ASSERT_TRUE(parsedRequest.get_body().empty());
}

TEST(HttpRequestTest, GetHeaderExisting)
{
  std::map<std::string, std::string> headers = {
    {"Content-Type", "application/json"},
    {"Authorization", "Bearer token"}
  };
  pine::http_request request(pine::http_method::get, "/api/users", pine::http_version::http_1_1, headers, "");

  std::string headerValue = request.get_header("Content-Type");

  ASSERT_EQ("application/json", headerValue);
}

TEST(HttpRequestTest, GetHeaderNonExisting)
{
  std::map<std::string, std::string> headers = {
    {"Authorization", "Bearer token"},
    {"Content-Type", "application/json"}
  };
  pine::http_request request(pine::http_method::get, "/api/users", pine::http_version::http_1_1, headers, "");

  std::string headerValue = request.get_header("Accept");

  ASSERT_TRUE(headerValue.empty());
}

TEST(HttpRequestTest, ToString)
{
  std::map<std::string, std::string> headers = {
    {"Authorization", "Bearer token"},
    {"Content-Type", "application/json"}
  };
  pine::http_request request(pine::http_method::post, "/api/users", pine::http_version::http_1_1, headers, "Hello World");

  std::string expectedString = "POST /api/users HTTP/1.1\r\n"
    "Authorization: Bearer token\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "Hello World";

  std::string toStringResult = request.to_string();

  ASSERT_EQ(expectedString, toStringResult);
}
