#include <gtest/gtest.h>
#include "http_response.h"

TEST(HttpResponseTest, Parse_ValidResponse_ReturnsExpectedValues)
{
  std::string response = "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 100\r\n"
    "\r\n"
    "<html><body>Hello, World!</body></html>";

  std::error_code ec;
  pine::http_response result = pine::http_response::parse(response, ec);

  ASSERT_FALSE(ec);
  EXPECT_EQ(pine::http_version::http_1_1, result.get_version());
  EXPECT_EQ(pine::http_status::ok, result.get_status());
  EXPECT_EQ("text/html", result.get_header("Content-Type"));
  EXPECT_EQ("100", result.get_header("Content-Length"));
  EXPECT_EQ("<html><body>Hello, World!</body></html>", result.get_body());
}

TEST(HttpResponseTest, Parse_InvalidResponse_ReturnsEmptyResponse)
{
  std::string response = "Invalid Response";

  std::error_code ec;
  pine::http_response result = pine::http_response::parse(response, ec);

  ASSERT_TRUE(ec);
  EXPECT_EQ(pine::http_version::http_1_1, result.get_version());
  EXPECT_EQ(pine::http_status::ok, result.get_status());
  EXPECT_EQ("", result.get_header("Content-Type"));
  EXPECT_EQ("", result.get_header("Content-Length"));
  EXPECT_EQ("", result.get_body());
}

TEST(HttpResponseTest, GetHeader_ValidHeader_ReturnsExpectedValue)
{
  std::string response = "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 100\r\n"
    "\r\n"
    "<html><body>Hello, World!</body></html>";

  std::error_code ec;
  pine::http_response result = pine::http_response::parse(response, ec);

  ASSERT_FALSE(ec);
  EXPECT_EQ("text/html", result.get_header("Content-Type"));
  EXPECT_EQ("100", result.get_header("Content-Length"));
}

TEST(HttpResponseTest, GetHeader_InvalidHeader_ReturnsEmptyString)
{
  std::string response = "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 100\r\n"
    "\r\n"
    "<html><body>Hello, World!</body></html>";

  std::error_code ec;
  pine::http_response result = pine::http_response::parse(response, ec);

  ASSERT_FALSE(ec);
  EXPECT_EQ("", result.get_header("Invalid-Header"));
}

TEST(HttpResponseTest, ToString_ReturnsExpectedString)
{
  std::string response = "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Content-Length: 100\r\n"
    "\r\n"
    "<html><body>Hello, World!</body></html>";

  std::error_code ec;
  pine::http_response result = pine::http_response::parse(response, ec);

  ASSERT_FALSE(ec);

  std::string expectedString = "HTTP/1.1 200 OK\r\n"
    "Content-Length: 100\r\n"
    "Content-Type: text/html\r\n"
    "\r\n"
    "<html><body>Hello, World!</body></html>";

  EXPECT_EQ(expectedString, result.to_string());
}
