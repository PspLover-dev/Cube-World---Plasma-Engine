#pragma once

#include <stdexcept>
#include <string>

namespace plasma {

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& msg) : std::runtime_error(msg) {}
};

class InvalidFileFormatException : public Exception {
public:
    explicit InvalidFileFormatException(const std::string& msg = "Invalid file format")
        : Exception(msg) {}
};

class InvalidVersionException : public Exception {
public:
    explicit InvalidVersionException(const std::string& msg = "Invalid version")
        : Exception(msg) {}
};

class InvalidDemoLicenseException : public Exception {
public:
    explicit InvalidDemoLicenseException(const std::string& msg = "Invalid demo license")
        : Exception(msg) {}
};

} // namespace plasma
