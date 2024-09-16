#include <gtest/gtest.h>
#include "error.h"
#include "http_request.h"

TEST(HttpRequestTests, DefaultConstructor)
{
  pine::http_request request;
  EXPECT_EQ(pine::http_method::get, request.get_method());
  EXPECT_EQ("", request.get_uri());
  EXPECT_EQ(pine::http_version::http_1_1, request.get_version());
  EXPECT_TRUE(request.get_headers().empty());
  EXPECT_EQ("", request.get_body());
}

TEST(HttpRequestTests, ParameterizedConstructor)
{
  pine::http_request request(pine::http_method::post, "/api/users", pine::http_version::http_1_1,
                             {{"Content-Type", "application/json"}, {"Authorization", "Bearer token"}},
                             R"({"name": "John Doe", "age": 30})");

  EXPECT_EQ(pine::http_method::post, request.get_method());
  EXPECT_EQ("/api/users", request.get_uri());
  EXPECT_EQ(pine::http_version::http_1_1, request.get_version());
  EXPECT_EQ(2, request.get_headers().size());
  EXPECT_EQ("application/json", request.get_header("Content-Type"));
  EXPECT_EQ("Bearer token", request.get_header("Authorization"));
  EXPECT_EQ(R"({"name": "John Doe", "age": 30})", request.get_body());
}

TEST(HttpRequestTests, SettersAndGetters)
{
  pine::http_request request;

  request.set_method(pine::http_method::post);
  request.set_uri("/api/products");
  request.set_version(pine::http_version::http_1_1);
  request.set_header("Content-Type", "application/xml");
  request.set_body("<product><name>Widget</name><price>9.99</price></product>");

  EXPECT_EQ(pine::http_method::post, request.get_method());
  EXPECT_EQ("/api/products", request.get_uri());
  EXPECT_EQ(pine::http_version::http_1_1, request.get_version());
  EXPECT_EQ(1, request.get_headers().size());
  EXPECT_EQ("application/xml", request.get_header("Content-Type"));
  EXPECT_EQ("<product><name>Widget</name><price>9.99</price></product>", request.get_body());
}

TEST(HttpRequestTests, ParseValidRequest)
{
  std::string requestStr = "GET /api/users HTTP/1.1\r\n"
                           "Host: example.com\r\n"
                           "User-Agent: Mozilla/5.0\r\n"
                           "Accept: application/json\r\n"
                           "\r\n";

  auto result = pine::http_request::parse(requestStr);
  ASSERT_TRUE(result.has_value());

  pine::http_request request = result.value();

  EXPECT_EQ(pine::http_method::get, request.get_method());
  EXPECT_EQ("/api/users", request.get_uri());
  EXPECT_EQ(pine::http_version::http_1_1, request.get_version());
  EXPECT_EQ(3, request.get_headers().size());
  EXPECT_EQ("example.com", request.get_header("Host"));
  EXPECT_EQ("Mozilla/5.0", request.get_header("User-Agent"));
  EXPECT_EQ("application/json", request.get_header("Accept"));
  EXPECT_EQ("", request.get_body());
}

TEST(HttpRequestTests, ParseInvalidRequest)
{
  std::string requestStr = "INVALID REQUEST";

  auto result = pine::http_request::parse(requestStr);
  ASSERT_FALSE(result.has_value());

  std::error_code error = result.error();
  EXPECT_EQ(pine::make_error_code(pine::error::parse_error_method), error);
}
