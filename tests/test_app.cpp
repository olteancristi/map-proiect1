#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <chrono>

#include "app.hpp"

using namespace map_project;

TEST_CASE("health body reports ok and uptime") {
    const auto started_at = std::chrono::steady_clock::now();
    const auto body = HealthBody(started_at);

    CHECK(body["status"] == "ok");
    CHECK(body.contains("uptime_seconds"));
    CHECK(body["uptime_seconds"].get<long>() >= 0);
}

TEST_CASE("version body contains all required fields") {
    const auto body = VersionBody();

    CHECK(body["app"] == kAppName);
    CHECK(body["version"] == kAppVersion);
    CHECK(body.contains("commit"));
    CHECK(body.contains("built_at"));
}

TEST_CASE("env falls back when the variable is missing") {
    CHECK(Env("MAP_VARIABLE_THAT_DOES_NOT_EXIST", "fallback") == "fallback");
}

TEST_CASE("error body has the standard shape") {
    const auto body = ErrorBody("not_found", "route does not exist");

    CHECK(body["error"] == "not_found");
    CHECK(body["message"] == "route does not exist");
}

TEST_CASE("new store starts at one") {
    Store store;
    CHECK(store.NextId() == 1);
}

TEST_CASE("take next id increments") {
    Store store;

    CHECK(store.TakeNextId() == 1);
    CHECK(store.TakeNextId() == 2);
    CHECK(store.NextId() == 3);
}

TEST_CASE("reset returns to one") {
    Store store;
    store.TakeNextId();
    store.TakeNextId();

    store.Reset();

    CHECK(store.NextId() == 1);
}

TEST_CASE("home page mentions the application name") {
    CHECK(HomePage().find(kAppName) != std::string::npos);
}


TEST_CASE("valid name is accepted") {
    CHECK(IsValidName("Ana Pop"));
}

TEST_CASE("empty name is rejected") {
    CHECK_FALSE(IsValidName(""));
}

TEST_CASE("name longer than 50 characters is rejected") {
    CHECK_FALSE(IsValidName(std::string(51, 'a')));
}

TEST_CASE("valid email is accepted") {
    CHECK(IsValidEmail("ana@example.com"));
}

TEST_CASE("email without at is rejected") {
    CHECK_FALSE(IsValidEmail("anaexample.com"));
}

TEST_CASE("email with multiple at signs is rejected") {
    CHECK_FALSE(IsValidEmail("ana@@example.com"));
}

TEST_CASE("valid phone is accepted") {
    CHECK(IsValidPhone("0722334455"));
}

TEST_CASE("phone with optional plus is accepted") {
    CHECK(IsValidPhone("+0722334455"));
}

TEST_CASE("invalid category is rejected") {
    CHECK_FALSE(IsValidCategory("school"));
}

TEST_CASE("email lookup is case insensitive") {
    Store store;

    store.AddContact(
        "Ana Pop",
        "Ana@Example.com",
        "0722334455",
        "work"
    );

    CHECK(store.EmailExists("ana@example.com"));
    CHECK(store.EmailExists("ANA@EXAMPLE.COM"));
}


TEST_CASE("add contact assigns id and stores data") {
    Store store;

    const auto contact = store.AddContact(
        "Ana Pop",
        "ana@example.com",
        "0722334455",
        "work"
    );

    CHECK(contact.id == 1);
    CHECK(contact.name == "Ana Pop");
    CHECK(contact.email == "ana@example.com");
    CHECK(contact.phone == "0722334455");
    CHECK(contact.category == "work");
}

TEST_CASE("get contact by id finds existing contact") {
    Store store;

    store.AddContact(
        "Ana Pop",
        "ana@example.com",
        "0722334455",
        "work"
    );

    const auto contact = store.GetContactById(1);

    REQUIRE(contact.has_value());
    CHECK(contact->name == "Ana Pop");
}

TEST_CASE("get contact by id returns empty for missing contact") {
    Store store;

    const auto contact = store.GetContactById(999);

    CHECK_FALSE(contact.has_value());
}

TEST_CASE("delete contact removes it") {
    Store store;

    store.AddContact(
        "Ana Pop",
        "ana@example.com",
        "0722334455",
        "work"
    );

    CHECK(store.DeleteContactById(1));
    CHECK_FALSE(store.GetContactById(1).has_value());
}