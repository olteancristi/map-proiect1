// Starter skeleton for the MAP project.
// Pure logic lives here so it can be unit tested without starting a server.
#pragma once

#include <chrono>
#include <cstdlib>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <optional>
#include <nlohmann/json.hpp>

namespace map_project {

inline constexpr const char* kAppName = "map-project";
inline constexpr const char* kAppVersion = "0.1.0";

/// Reads an environment variable, falling back when it is missing or empty.
inline std::string Env(const char* key, const std::string& fallback) {
    const char* value = std::getenv(key);
    return (value == nullptr || *value == '\0') ? fallback : std::string(value);
}


inline bool IsValidName(const std::string& name) {
    return !name.empty() && name.size() <= 50;
}

inline bool IsValidEmail(const std::string& email) {
    const auto at = email.find('@');

    if (at == std::string::npos || at == 0) {
        return false;
    }

    if (email.find('@', at + 1) != std::string::npos) {
        return false;
    }

    return email.find('.', at + 1) != std::string::npos;
}

inline bool IsValidPhone(const std::string& phone) {
    std::string digits = phone;

    if (!digits.empty() && digits[0] == '+') {
        digits.erase(0, 1);
    }

    return digits.size() == 10 &&
           std::all_of(digits.begin(), digits.end(),
                       [](unsigned char c) { return std::isdigit(c); });
}

inline bool IsValidCategory(const std::string& category) {
    return category == "family" ||
           category == "friends" ||
           category == "work" ||
           category == "other";
}

inline std::string ToLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) {
                       return static_cast<char>(std::tolower(c));
                   });
    return value;
}


struct Contact {
    int id;
    std::string name;
    std::string email;
    std::string phone;
    std::string category;
};
/// Holds the application data in memory.
/// Everything is lost when the container restarts. Add your theme's structures here.
class Store {
public:
   void Reset() {
    std::lock_guard<std::mutex> guard(mutex_);
    contacts_.clear();
    next_id_ = 1;
}

    int NextId() {
        std::lock_guard<std::mutex> guard(mutex_);
        return next_id_;
    }

    int TakeNextId() {
        std::lock_guard<std::mutex> guard(mutex_);
        return next_id_++;
    }

    bool EmailExists(const std::string& email) {
    std::lock_guard<std::mutex> guard(mutex_);

    const std::string normalized = ToLower(email);

    return std::any_of(
        contacts_.begin(),
        contacts_.end(),
        [&](const Contact& contact) {
            return ToLower(contact.email) == normalized;
        }
    );
}

    Contact AddContact(const std::string& name,
                   const std::string& email,
                   const std::string& phone,
                   const std::string& category) {
    std::lock_guard<std::mutex> guard(mutex_);

    Contact contact{
        next_id_++,
        name,
        email,
        phone,
        category
    };

    contacts_.push_back(contact);
    return contact;
}


std::vector<Contact> GetContacts() {
    std::lock_guard<std::mutex> guard(mutex_);
    return contacts_;
}


std::optional<Contact> GetContactById(int id) {
    std::lock_guard<std::mutex> guard(mutex_);

    const auto it = std::find_if(
        contacts_.begin(),
        contacts_.end(),
        [id](const Contact& contact) {
            return contact.id == id;
        }
    );

    if (it == contacts_.end()) {
        return std::nullopt;
    }

    return *it;
}


bool DeleteContactById(int id) {
    std::lock_guard<std::mutex> guard(mutex_);

    const auto it = std::find_if(
        contacts_.begin(),
        contacts_.end(),
        [id](const Contact& contact) {
            return contact.id == id;
        }
    );

    if (it == contacts_.end()) {
        return false;
    }

    contacts_.erase(it);
    return true;
}


private:
    std::mutex mutex_;
    int next_id_ = 1;
    std::vector<Contact> contacts_;
    // example: std::map<int, Contact> contacts_;
};

/// Seconds since the process started.
inline long UptimeSeconds(std::chrono::steady_clock::time_point started_at) {
    const auto elapsed = std::chrono::steady_clock::now() - started_at;
    return std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
}

inline nlohmann::json HealthBody(std::chrono::steady_clock::time_point started_at) {
    return {{"status", "ok"}, {"uptime_seconds", UptimeSeconds(started_at)}};
}

inline nlohmann::json VersionBody() {
    return {
        {"app", kAppName},
        {"version", kAppVersion},
        {"commit", Env("APP_COMMIT", "dev")},
        {"built_at", Env("APP_BUILT_AT", "unknown")},
    };
}

inline nlohmann::json ErrorBody(const std::string& code, const std::string& message) {
    return {{"error", code}, {"message", message}};
}

inline std::string HomePage() {
    return std::string(R"(<!DOCTYPE html>
<html lang="ro"><head><meta charset="utf-8"><title>)") + kAppName + R"(</title></head>
<body>
<h1>)" + kAppName + R"(</h1>
<p>Autor: Oltean Cristian-Daniel 2.1</p>
<p>Tema: 1 - Agenda de contacte</p>
<p>Versiune: )" + kAppVersion + ", commit " + Env("APP_COMMIT", "dev") + R"(</p>
</body></html>)";
}

}  // namespace map_project
