#pragma once

#include <string>

namespace plasma {

class FloatFilter {
public:
    static bool accept(const std::wstring& text, float& out);
};

class IntegerFilter {
public:
    static bool accept(const std::wstring& text, int& out);
};

class UnsignedFloatFilter {
public:
    static bool accept(const std::wstring& text, float& out);
};

class UnsignedIntegerFilter {
public:
    static bool accept(const std::wstring& text, unsigned& out);
};

} // namespace plasma
