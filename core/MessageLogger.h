#pragma once

#include <iostream>
#include <string>

namespace core {

class MessageLogger {
public:
    enum messageType {
        Info,
        Warning,
        Error,
    };
public:
    static auto Log(messageType type, std::string message) -> void {
        std::cerr << GetTypeString(type) << ":\t" << message << std::endl;
    }
private:
    static auto GetTypeString(messageType type) -> std::string {
        auto typeString = std::string{};
        switch (type) {
        default:
            return "Unknown";
        case messageType::Info:
            typeString = "Info";
            return typeString;
        case messageType::Warning:
            typeString = "Warning";
            return typeString;
        case messageType::Error:
            typeString = "Error";
            return typeString;
        }
    }
};

}