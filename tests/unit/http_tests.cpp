#include <doctest/doctest.h>

#include "error.h"
#include "http.h"

using namespace pine;

TEST_SUITE("HTTP Tests")
{
  TEST_CASE("http::http_method_strings")
  {
    CHECK("GET" == http_method_strings.at(http_method::get));
    CHECK("HEAD" == http_method_strings.at(http_method::head));
    CHECK("POST" == http_method_strings.at(http_method::post));
    CHECK("PUT" == http_method_strings.at(http_method::put));
    CHECK("DELETE" == http_method_strings.at(http_method::delete_));
    CHECK("CONNECT" == http_method_strings.at(http_method::connect));
    CHECK("OPTIONS" == http_method_strings.at(http_method::options));
    CHECK("TRACE" == http_method_strings.at(http_method::trace));
    CHECK("PATCH" == http_method_strings.at(http_method::patch));
  }

  TEST_CASE("http::http_status_strings")
  {
    CHECK("OK" == http_status_strings.at(http_status::ok));
    CHECK("Bad Request" == http_status_strings.at(http_status::bad_request));
    CHECK("Not Found" == http_status_strings.at(http_status::not_found));
    CHECK("Internal Server Error" == http_status_strings.at(http_status::internal_server_error));
    CHECK("Method Not Allowed" == http_status_strings.at(http_status::method_not_allowed));
  }

  TEST_CASE("http::http_version_strings")
  {
    CHECK("HTTP/1.1" == http_version_strings.at(http_version::http_1_1));
  }

  TEST_CASE("http_utils::try_get_body")
  {
    std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\nHello World";
    size_t offset = 47;
    auto result = http_utils::try_get_body(request, offset);
    CHECK(result.has_value());
    CHECK("Hello World" == result.value());
  }

  TEST_CASE("http_utils::try_get_headers")
  {
    std::string request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/html\r\n\r\n";
    size_t offset = 26;
    auto result = http_utils::try_get_headers(request, offset);
    CHECK(result.has_value());
    CHECK(2 == result.value().size());
    CHECK("example.com" == result.value().at("Host"));
    CHECK("text/html" == result.value().at("Content-Type"));
  }

  TEST_CASE("http_utils::try_get_header")
  {
    std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    size_t offset = 26;
    auto result = http_utils::try_get_header(request, offset);
    CHECK(result.has_value());
    CHECK("Host" == result.value().first);
    CHECK("example.com" == result.value().second);
  }

  TEST_CASE("http_utils::try_get_method")
  {
    std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    size_t offset = 0;
    auto result = http_utils::try_get_method(request, offset);
    CHECK(result.has_value());
    CHECK(http_method::get == result.value());
  }

  TEST_CASE("http_utils::try_get_status")
  {
    std::string_view response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    size_t offset = 9;
    auto result = http_utils::try_get_status(response, offset);
    CHECK(result.has_value());
    CHECK(http_status::ok == result.value());
  }

  TEST_CASE("http_utils::try_get_uri")
  {
    std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    size_t offset = 4;
    auto result = http_utils::try_get_uri(request, offset);
    CHECK(result.has_value());
    CHECK("/index.html" == result.value());
  }

  TEST_CASE("http_utils::try_get_version")
  {
    std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    size_t offset = 16;
    auto result = http_utils::try_get_version(request, offset);
    CHECK(result.has_value());
    CHECK(http_version::http_1_1 == result.value());
  }
}
