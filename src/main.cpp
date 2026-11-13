// Wires the HTTP routes of the common contract.
// Add the routes required by your assigned theme below.

#include <chrono>
#include <iostream>

#include <httplib.h>

#include "app.hpp"

int main() {
    using namespace map_project;

    const auto started_at = std::chrono::steady_clock::now();
    Store store;
    httplib::Server server;

    server.Get("/health", [&](const httplib::Request&, httplib::Response& response) {
        response.set_content(HealthBody(started_at).dump(), "application/json; charset=utf-8");
    });

    server.Get("/version", [](const httplib::Request&, httplib::Response& response) {
        response.set_content(VersionBody().dump(), "application/json; charset=utf-8");
    });

    server.Post("/reset", [&](const httplib::Request&, httplib::Response& response) {
        store.Reset();
        response.status = 204;
    });

    server.Post("/contacts", [&](const httplib::Request& request, httplib::Response& response) {
    try {
        const auto body = nlohmann::json::parse(request.body);

        if (!body.contains("name") ||
            !body.contains("email") ||
            !body.contains("phone") ||
            !body.contains("category") ||
            !body["name"].is_string() ||
            !body["email"].is_string() ||
            !body["phone"].is_string() ||
            !body["category"].is_string()) {

            response.status = 400;
            response.set_content(
                ErrorBody("validation", "missing or invalid fields").dump(),
                "application/json; charset=utf-8"
            );
            return;
        }

        const std::string name = body["name"].get<std::string>();
        const std::string email = body["email"].get<std::string>();
        const std::string phone = body["phone"].get<std::string>();
        const std::string category = body["category"].get<std::string>();

        if (!IsValidName(name) ||
            !IsValidEmail(email) ||
            !IsValidPhone(phone) ||
            !IsValidCategory(category)) {

            response.status = 400;
            response.set_content(
                ErrorBody("validation", "invalid contact data").dump(),
                "application/json; charset=utf-8"
            );
            return;
        }

        if (store.EmailExists(email)) {
            response.status = 409;
            response.set_content(
                ErrorBody("conflict", "email already exists").dump(),
                "application/json; charset=utf-8"
            );
            return;
        }

        const Contact contact = store.AddContact(name, email, phone, category);

        nlohmann::json result = {
            {"id", contact.id},
            {"name", contact.name},
            {"email", contact.email},
            {"phone", contact.phone},
            {"category", contact.category}
        };

        response.status = 201;
        response.set_content(
            result.dump(),
            "application/json; charset=utf-8"
        );

    } catch (const nlohmann::json::exception&) {
        response.status = 400;
        response.set_content(
            ErrorBody("validation", "invalid JSON").dump(),
            "application/json; charset=utf-8"
        );
    }
});



server.Get("/contacts", [&](const httplib::Request& request, httplib::Response& response) {
    auto contacts = store.GetContacts();

    if (request.has_param("category")) {
        const std::string category = request.get_param_value("category");

        contacts.erase(
            std::remove_if(
                contacts.begin(),
                contacts.end(),
                [&](const Contact& contact) {
                    return contact.category != category;
                }
            ),
            contacts.end()
        );
    }

    if (request.has_param("q")) {
        const std::string query = ToLower(request.get_param_value("q"));

        contacts.erase(
            std::remove_if(
                contacts.begin(),
                contacts.end(),
                [&](const Contact& contact) {
                    return ToLower(contact.name).find(query) == std::string::npos;
                }
            ),
            contacts.end()
        );
    }

    std::sort(
        contacts.begin(),
        contacts.end(),
        [](const Contact& a, const Contact& b) {
            if (a.name == b.name) {
                return a.id < b.id;
            }

            return a.name < b.name;
        }
    );

    nlohmann::json result = nlohmann::json::array();

    for (const auto& contact : contacts) {
        result.push_back({
            {"id", contact.id},
            {"name", contact.name},
            {"email", contact.email},
            {"phone", contact.phone},
            {"category", contact.category}
        });
    }

    response.status = 200;
    response.set_content(
        result.dump(),
        "application/json; charset=utf-8"
    );
});



server.Get(R"(/contacts/(\d+))", [&](const httplib::Request& request, httplib::Response& response) {
    const int id = std::stoi(request.matches[1].str());

    const auto contact = store.GetContactById(id);

    if (!contact.has_value()) {
        response.status = 404;
        response.set_content(
            ErrorBody("not_found", "contact not found").dump(),
            "application/json; charset=utf-8"
        );
        return;
    }

    nlohmann::json result = {
        {"id", contact->id},
        {"name", contact->name},
        {"email", contact->email},
        {"phone", contact->phone},
        {"category", contact->category}
    };

    response.status = 200;
    response.set_content(
        result.dump(),
        "application/json; charset=utf-8"
    );
});


server.Delete(R"(/contacts/(\d+))", [&](const httplib::Request& request, httplib::Response& response) {
    const int id = std::stoi(request.matches[1].str());

    if (!store.DeleteContactById(id)) {
        response.status = 404;
        response.set_content(
            ErrorBody("not_found", "contact not found").dump(),
            "application/json; charset=utf-8"
        );
        return;
    }

    response.status = 204;
});



server.Get("/stats", [&](const httplib::Request&, httplib::Response& response) {
    const auto contacts = store.GetContacts();

    nlohmann::json by_category = nlohmann::json::object();

    for (const auto& contact : contacts) {
        if (!by_category.contains(contact.category)) {
            by_category[contact.category] = 0;
        }

        by_category[contact.category] =
            by_category[contact.category].get<int>() + 1;
    }

    nlohmann::json result = {
        {"total", contacts.size()},
        {"by_category", by_category}
    };

    response.status = 200;
    response.set_content(
        result.dump(),
        "application/json; charset=utf-8"
    );
});


    server.Get("/", [](const httplib::Request&, httplib::Response& response) {
        response.set_content(HomePage(), "text/html; charset=utf-8");
    });

    server.set_error_handler([](const httplib::Request&, httplib::Response& response) {
    if (response.status == 404 && response.body.empty()) {
        response.set_content(
            ErrorBody("not_found", "route does not exist").dump(),
            "application/json; charset=utf-8"
        );
    }
});

    std::cout << kAppName << " " << kAppVersion << " starting on port 8080" << std::endl;
    if (!server.listen("0.0.0.0", 8080)) {
        std::cerr << "failed to bind port 8080" << std::endl;
        return 1;
    }
    return 0;
}
