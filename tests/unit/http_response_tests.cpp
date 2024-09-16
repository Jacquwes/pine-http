#include <gtest/gtest.h>
#include "http_response.h"

using namespace pine;

TEST(HttpResponseTests, GetBody_ReturnsCorrectValue)
{
  http_response response;
  response.set_body("Hello, World!");

  EXPECT_EQ("Hello, World!", response.get_body());
}

TEST(HttpResponseTests, GetHeader_ReturnsCorrectValue)
{
  http_response response;
  response.set_header("Content-Type", "application/json");

  EXPECT_EQ("application/json", response.get_header("Content-Type"));
}

TEST(HttpResponseTests, GetHeaders_ReturnsCorrectValue)
{
  http_response response;
  response.set_header("Content-Type", "application/json");
  response.set_header("Content-Length", "100");

  const auto& headers = response.get_headers();

  EXPECT_EQ("application/json", headers.at("Content-Type"));
  EXPECT_EQ("100", headers.at("Content-Length"));
}

TEST(HttpResponseTests, GetStatus_ReturnsCorrectValue)
{
  http_response response;
  response.set_status(http_status::ok);

  EXPECT_EQ(http_status::ok, response.get_status());
}

TEST(HttpResponseTests, GetVersion_ReturnsCorrectValue)
{
  http_response response;
  response.set_version(http_version::http_1_1);

  EXPECT_EQ(http_version::http_1_1, response.get_version());
}

TEST(HttpResponseTests, ToString_ReturnsCorrectValue)
{
  http_response response;
  response.set_status(http_status::ok);
  response.set_version(http_version::http_1_1);
  response.set_header("Content-Type", "application/json");
  response.set_body("Hello, World!");

  std::string expected = "HTTP/1.1 200 OK\r\n";
  expected += "Content-Length: 13\r\n";
  expected += "Content-Type: application/json\r\n";
  expected += "\r\n";
  expected += "Hello, World!";

  EXPECT_EQ(expected, response.to_string());
}
