#include <gtest/gtest.h>
#include <map>
#include <string>
#include <string_view>
#include <system_error>
#include "http.h"

using namespace pine;
using namespace pine::http_utils;

TEST(HttpUtilsTest, FindBody)
{
  std::string_view request = "GET /api/users HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\"name\":\"John\",\"age\":30}";

  // index of the first character of the body
  size_t offset = 76;
  std::error_code ec;
  std::string body = find_body(request, offset, ec);

  EXPECT_EQ(ec, std::error_code{});
  EXPECT_EQ(body, "{\"name\":\"John\",\"age\":30}");
}

TEST(HttpUtilsTest, FindHeader)
{
  std::string_view request = "GET /api/users HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\"name\":\"John\",\"age\":30}";

  size_t offset = 0;
  std::error_code ec;
  auto [name, value] = find_header(request, offset, ec);

  EXPECT_EQ(ec, std::error_code{});
  EXPECT_EQ(name, "Host");
  EXPECT_EQ(value, "example.com");
}

TEST(HttpUtilsTest, FindHeaders)
{
  std::string request = "GET /api/users HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\"name\":\"John\",\"age\":30}";

  size_t offset = 0;
  std::error_code ec;
  std::map<std::string, std::string> headers = find_headers(request, offset, ec);

  EXPECT_EQ(ec, std::error_code{});
  EXPECT_EQ(headers.size(), 2);
  EXPECT_EQ(headers["Host"], "example.com");
  EXPECT_EQ(headers["Content-Type"], "application/json");
}

TEST(HttpUtilsTest, FindMethod)
{
  std::string_view request = "GET /api/users HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\"name\":\"John\",\"age\":30}";

  size_t offset = 0;
  std::error_code ec;
  http_method method = find_method(request, offset, ec);

  EXPECT_EQ(ec, std::error_code{});
  EXPECT_EQ(method, http_method::get);
}

TEST(HttpUtilsTest, FindStatus)
{
  std::string_view request = "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\"name\":\"John\",\"age\":30}";

  size_t offset = 0;
  std::error_code ec;
  http_status status = find_status(request, offset, ec);

  EXPECT_EQ(ec, std::error_code{});
  EXPECT_EQ(status, http_status::ok);
}

TEST(HttpUtilsTest, FindUri)
{
  std::string_view request = "GET /api/users HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\"name\":\"John\",\"age\":30}";

  size_t offset = 0;
  std::error_code ec;
  std::string uri = find_uri(request, offset, ec);

  EXPECT_EQ(ec, std::error_code{});
  EXPECT_EQ(uri, "/api/users");
}

TEST(HttpUtilsTest, FindVersion)
{
  std::string_view request = "GET /api/users HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Content-Type: application/json\r\n"
    "\r\n"
    "{\"name\":\"John\",\"age\":30}";

  size_t offset = 0;
  std::error_code ec;
  http_version version = find_version(request, offset, ec);

  EXPECT_EQ(ec, std::error_code{});
  EXPECT_EQ(version, http_version::http_1_1);
}
