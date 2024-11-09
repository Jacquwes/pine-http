#include <doctest/doctest.h>

#include <route_node.h>
#include <route_tree.h>

using namespace pine;

TEST_SUITE("Route Tests")
{
  TEST_CASE("route_node::route_node")
  {
    SUBCASE("Root node")
    {
      route_node node("/");
      CHECK("" == node.path());
    }

    SUBCASE("Root level node")
    {
      route_node node("/api");
      const route_node& api_node = *node.children().at(0).get();
      CHECK("api" == api_node.path());
    }

    SUBCASE("Deep level node")
    {
      route_node node("/api/users/messages");
      const route_node& api_node = *node.children().at(0).get();
      const route_node& users_node = *api_node.children().at(0).get();
      const route_node& messages_node = *users_node.children().at(0).get();
      CHECK("api" == api_node.path());
      CHECK("users" == users_node.path());
      CHECK("messages" == messages_node.path());
    }

    SUBCASE("Path parameter node")
    {
      route_node node("/api/users/:id");
      const route_node& api_node = *node.children().at(0).get();
      const route_node& users_node = *api_node.children().at(0).get();
      const route_node& id_node = *users_node.children().at(0).get();
      CHECK("api" == api_node.path());
      CHECK("users" == users_node.path());
      CHECK(":id" == id_node.path());
      CHECK(users_node.has_path_parameter_children());
      CHECK(id_node.is_path_parameter());
    }
  }

  TEST_CASE("route_node::add_child")
  {
    SUBCASE("Add two nodes to root")
    {
      route_node node("/");
      const route_node& api_node = node.add_child("api");
      const route_node& users_node = node.add_child("users");
      CHECK("api" == api_node.path());
      CHECK("users" == users_node.path());
    }

    SUBCASE("Add two nodes to root and one to api")
    {
      route_node node("/");
      auto& api_node = node.add_child("api");
      const auto& users_node = node.add_child("users");
      const auto& messages_node = api_node.add_child("messages");
      CHECK("api" == api_node.path());
      CHECK("users" == users_node.path());
      CHECK("messages" == messages_node.path());
    }

    SUBCASE("Add two path parameter children to same route")
    {
      route_node node("/");
      auto& api_node = node.add_child("api");
      api_node.add_child(":id");
      CHECK_THROWS(api_node.add_child(":name"));
    }
  }

  TEST_CASE("route_node::find_child")
  {
    SUBCASE("Find child")
    {
      route_node node("/api/users");
      const route_node& api_node = node.find_child("api");
      const route_node& users_node = api_node.find_child("users");
      CHECK("api" == api_node.path());
      CHECK("users" == users_node.path());
    }

    SUBCASE("Find child with path parameter")
    {
      route_node node("/api/users/:id");
      const route_node& api_node = node.find_child("api");
      const route_node& users_node = api_node.find_child("users");
      const route_node& id_node = users_node.find_child("api/users/:id");
      CHECK(":id" == id_node.path());
    }

    SUBCASE("Find child with similar path")
    {
      route_node node("/");
      node.add_child("api");
      node.add_child("api2");
      const route_node& api_node = node.find_child("api");
      const route_node& api2_node = node.find_child("api2");
      CHECK("api" == api_node.path());
      CHECK("api2" == api2_node.path());
    }
  }

  TEST_CASE("route_tree::add_route")
  {
    SUBCASE("Add route to root")
    {
      route_tree tree;
      tree.add_route(route_path("/api"));
      const route_node& api_node = *tree.root().children().at(0).get();
      CHECK("api" == api_node.path());
    }

    SUBCASE("Add route with path parameter")
    {
      route_tree tree;
      tree.add_route(route_path("/api/users/:id"));
      const route_node& api_node = *tree.root().children().at(0).get();
      const route_node& users_node = *api_node.children().at(0).get();
      const route_node& id_node = *users_node.children().at(0).get();
      CHECK("api" == api_node.path());
      CHECK("users" == users_node.path());
      CHECK(":id" == id_node.path());
      CHECK(users_node.has_path_parameter_children());
      CHECK(id_node.is_path_parameter());
    }

    SUBCASE("Add multiple routes with the same start of path")
    {
      route_tree tree;
      tree.add_route(route_path("/api/users"));
      tree.add_route(route_path("/api/messages"));
      const route_node& api_node = *tree.root().children().at(0).get();
      const route_node& users_node = *api_node.children().at(0).get();
      const route_node& messages_node = *api_node.children().at(1).get();
      CHECK("api" == api_node.path());
      CHECK("users" == users_node.path());
      CHECK("messages" == messages_node.path());
    }

    SUBCASE("Add multiple routes with the same path")
    {
      route_tree tree;
      const auto& api_node = tree.add_route(route_path("/api"));
      const auto& api_node2 = tree.add_route(route_path("/api"));
      CHECK("api" == api_node.path());
      CHECK(&api_node == &api_node2);
    }
  }

  TEST_CASE("route_tree::find_route")
  {
    SUBCASE("Find route")
    {
      route_tree tree;
      tree.add_route(route_path("/api/users"));
      const route_node& users_node = tree.find_route("/api/users");
      CHECK("users" == users_node.path());
    }

    SUBCASE("Find route with path parameter")
    {
      route_tree tree;
      tree.add_route(route_path("/api/users/:id"));
      const route_node& id_node = tree.find_route("/api/users/123");
      CHECK(":id" == id_node.path());
    }

    SUBCASE("Find route with similar path")
    {
      route_tree tree;
      tree.add_route(route_path("/api/users"));
      tree.add_route(route_path("/api/users2"));
      const route_node& users_node = tree.find_route("/api/users");
      const route_node& users2_node = tree.find_route("/api/users2");
      CHECK("users" == users_node.path());
      CHECK("users2" == users2_node.path());
    }

    SUBCASE("Find route with similar path and path parameter")
    {
      route_tree tree;
      tree.add_route(route_path("/api/users"));
      tree.add_route(route_path("/api/users/:id"));
      const route_node& users_node = tree.find_route("/api/users");
      const route_node& id_node = tree.find_route("/api/users/123");
      CHECK("users" == users_node.path());
      CHECK(":id" == id_node.path());
    }
  }
}
