#include <doctest/doctest.h>

#include "error.h"
#include "http.h"

using namespace pine;

TEST_SUITE("HTTP Tests")
{
  TEST_CASE("http::http_method_strings")
  {
    CHECK(http_method_strings.at(http_method::get).compare("GET") == 0);
    CHECK(http_method_strings.at(http_method::head).compare("HEAD") == 0);
    CHECK(http_method_strings.at(http_method::post).compare("POST") == 0);
    CHECK(http_method_strings.at(http_method::put).compare("PUT") == 0);
    CHECK(http_method_strings.at(http_method::delete_).compare("DELETE") == 0);
    CHECK(http_method_strings.at(http_method::connect).compare("CONNECT") == 0);
    CHECK(http_method_strings.at(http_method::options).compare("OPTIONS") == 0);
    CHECK(http_method_strings.at(http_method::trace).compare("TRACE") == 0);
    CHECK(http_method_strings.at(http_method::patch).compare("PATCH") == 0);
  }

  TEST_CASE("http::http_status_strings")
  {
    CHECK(http_status_strings.at(http_status::ok).compare("OK") == 0);
    CHECK(http_status_strings.at(http_status::bad_request).compare("Bad Request") == 0);
    CHECK(http_status_strings.at(http_status::not_found).compare("Not Found") == 0);
    CHECK(http_status_strings.at(http_status::internal_server_error).compare("Internal Server Error") == 0);
    CHECK(http_status_strings.at(http_status::method_not_allowed).compare("Method Not Allowed") == 0);
  }

  TEST_CASE("http::http_version_strings")
  {
    CHECK(http_version_strings.at(http_version::http_1_1).compare("HTTP/1.1") == 0);
  }

  TEST_CASE("http_utils::try_get_body")
  {
    std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\nHello World";
    size_t offset = 47;
    auto result = http_utils::try_get_body(request, offset);
    CHECK(result.has_value());
    CHECK(result.value().compare("Hello World") == 0);
  }

  TEST_CASE("http_utils::try_get_headers")
  {
    std::string request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\nContent-Type: text/html\r\n\r\n";
    size_t offset = 26;
    auto result = http_utils::try_get_headers(request, offset);
    CHECK(result.has_value());
    CHECK(2 == result.value().size());
    CHECK(result.value().at("Host").compare("example.com") == 0);
    CHECK(result.value().at("Content-Type").compare("text/html") == 0);
  }

  TEST_CASE("http_utils::try_get_header")
  {
    std::string_view request = "GET /index.html HTTP/1.1\r\nHost: example.com\r\n\r\n";
    size_t offset = 26;
    auto result = http_utils::try_get_header(request, offset);
    CHECK(result.has_value());
    CHECK(result.value().first.compare("Host") == 0);
    CHECK(result.value().second.compare("example.com") == 0);
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
    CHECK(result.value().compare("/index.html") == 0);
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
