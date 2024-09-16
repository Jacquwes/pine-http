#include <gtest/gtest.h>
#include "error.h"
#include "http.h"

using namespace pine;

TEST(HttpTests, HttpMethodStrings)
{
  EXPECT_EQ("GET", http_method_strings.at(http_method::get));
  EXPECT_EQ("HEAD", http_method_strings.at(http_method::head));
  EXPECT_EQ("POST", http_method_strings.at(http_method::post));
}

TEST(HttpTests, HttpStatusStrings)
{
  EXPECT_EQ("OK", http_status_strings.at(http_status::ok));
  EXPECT_EQ("Bad Request", http_status_strings.at(http_status::bad_request));
  EXPECT_EQ("Not Found", http_status_strings.at(http_status::not_found));
  EXPECT_EQ("Internal Server Error", http_status_strings.at(http_status::internal_server_error));
}

TEST(HttpTests, HttpVersionStrings)
{
  EXPECT_EQ("HTTP/1.1", http_version_strings.at(http_version::http_1_1));
}

TEST(HttpUtilsTests, TryGetBody)
{
  std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\nHello World";
  size_t offset = 47;
  auto result = http_utils::try_get_body(request, offset);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("Hello World", result.value());
}

TEST(HttpUtilsTests, TryGetHeaders)
{
  std::string request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/html\r\n\r\n";
  size_t offset = 26;
  auto result = http_utils::try_get_headers(request, offset);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(2, result.value().size());
  EXPECT_EQ("example.com", result.value().at("Host"));
  EXPECT_EQ("text/html", result.value().at("Content-Type"));
}

TEST(HttpUtilsTests, TryGetHeader)
{
  std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
  size_t offset = 26;
  auto result = http_utils::try_get_header(request, offset);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("Host", result.value().first);
  EXPECT_EQ("example.com", result.value().second);
}

TEST(HttpUtilsTests, TryGetMethod)
{
  std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
  size_t offset = 0;
  auto result = http_utils::try_get_method(request, offset);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(http_method::get, result.value());
}

TEST(HttpUtilsTests, TryGetStatus)
{
  std::string_view response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  size_t offset = 9;
  auto result = http_utils::try_get_status(response, offset);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(http_status::ok, result.value());
}

TEST(HttpUtilsTests, TryGetUri)
{
  std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
  size_t offset = 4;
  auto result = http_utils::try_get_uri(request, offset);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ("/index.html", result.value());
}

TEST(HttpUtilsTests, TryGetVersion)
{
  std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
  size_t offset = 16;
  auto result = http_utils::try_get_version(request, offset);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(http_version::http_1_1, result.value());
}
