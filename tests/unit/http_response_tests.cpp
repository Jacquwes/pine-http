#include <doctest/doctest.h>
#include <pine/http_response.h>

using namespace pine;

TEST_SUITE("HTTP Response")
{
  TEST_CASE("http_response::set_body_and_get_body")
  {
    http_response response;
    response.set_body("Hello, World!");

    CHECK(response.get_body().compare("Hello, World!") == 0);
  }

  TEST_CASE("http_response::set_header_and_get_header")
  {
    http_response response;
    response.set_header("Content-Type", "application/json");

    CHECK(response.get_header("Content-Type").compare("application/json") == 0);
  }

  TEST_CASE("http_response::set_headers_and_get_headers")
  {
    http_response response;
    response.set_header("Content-Type", "application/json");
    response.set_header("Content-Length", "100");

    const auto& headers = response.get_headers();

    CHECK(headers.at("Content-Type").compare("application/json") == 0);
    CHECK(headers.at("Content-Length").compare("100") == 0);
  }

  TEST_CASE("http_response::set_status_and_get_status")
  {
    http_response response;
    response.set_status(http_status::ok);

    CHECK(http_status::ok == response.get_status());
  }

  TEST_CASE("http_response::set_version_and_get_version")
  {
    http_response response;
    response.set_version(http_version::http_1_1);

    CHECK(http_version::http_1_1 == response.get_version());
  }

  TEST_CASE("http_response::to_string")
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

    CHECK(expected == response.to_string());
  }
}
