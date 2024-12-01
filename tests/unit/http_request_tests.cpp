#include <doctest/doctest.h>
#include <pine/error.h>
#include <pine/http_request.h>

TEST_SUITE("HTTP Request")
{
  TEST_CASE("http_request::http_request")
  {
    SUBCASE("Default constructor")
    {
      pine::http_request request;
      CHECK(pine::http_method::get == request.get_method());
      CHECK(request.get_uri().compare("") == 0);
      CHECK(pine::http_version::http_1_1 == request.get_version());
      CHECK(request.get_headers().empty());
      CHECK(request.get_body().compare("") == 0);
    }

    SUBCASE("Constructor with parameters")
    {
      pine::http_request request(pine::http_method::post, "/api/users", pine::http_version::http_1_1,
                                 { {"Content-Type", "application/json"}, {"Authorization", "Bearer token"} },
                                 R"({"name": "John Doe", "age": 30})");

      CHECK(pine::http_method::post == request.get_method());
      CHECK(request.get_uri().compare("/api/users") == 0);
      CHECK(pine::http_version::http_1_1 == request.get_version());
      CHECK(2 == request.get_headers().size());
      CHECK(request.get_header("Content-Type").compare("application/json") == 0);
      CHECK(request.get_header("Authorization").compare("Bearer token") == 0);
      CHECK(request.get_body().compare(R"({"name": "John Doe", "age": 30})") == 0);
    }
  }

  TEST_CASE("http_request::(set_.*|get_.*)")
  {
    pine::http_request request;

    request.set_method(pine::http_method::post);
    request.set_uri("/api/products");
    request.set_version(pine::http_version::http_1_1);
    request.set_header("Content-Type", "application/xml");
    request.set_body("<product><name>Widget</name><price>9.99</price></product>");

    CHECK(pine::http_method::post == request.get_method());
    CHECK(request.get_uri().compare("/api/products") == 0);
    CHECK(pine::http_version::http_1_1 == request.get_version());
    // 2 headers: Content-Size is added automatically.
    CHECK(2 == request.get_headers().size());
    CHECK(request.get_header("Content-Type").compare("application/xml") == 0);
    CHECK(request.get_body().compare("<product><name>Widget</name><price>9.99</price></product>") == 0);
  }

  TEST_CASE("http_request::parse")
  {
    SUBCASE("Valid request")
    {
      std::string requestStr = "GET /api/users HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "Accept: application/json\r\n"
        "\r\n";

      auto result = pine::http_request::parse(requestStr);
      CHECK(result.has_value());

      pine::http_request request = result.value();

      CHECK(pine::http_method::get == request.get_method());
      CHECK(request.get_uri().compare("/api/users") == 0);
      CHECK(pine::http_version::http_1_1 == request.get_version());
      CHECK(3 == request.get_headers().size());
      CHECK(request.get_header("Host").compare("example.com") == 0);
      CHECK(request.get_header("User-Agent").compare("Mozilla/5.0") == 0);
      CHECK(request.get_header("Accept").compare("application/json") == 0);
      CHECK(request.get_body().compare("") == 0);
    }

    SUBCASE("Invalid request")
    {
      std::string requestStr = "INVALID REQUEST";

      auto result = pine::http_request::parse(requestStr);
      CHECK(!result.has_value());

      pine::error error = result.error();
      CHECK(pine::error_code::parse_error_method == error.code());
    }
  }
}