#include "plasma/Filters.hpp"

#include <cwchar>

namespace plasma {

bool FloatFilter::accept(const std::wstring& text, float& out) {
    if (text.empty()) {
        return false;
    }
    wchar_t* end = nullptr;
    out = std::wcstof(text.c_str(), &end);
    return end && *end == L'\0';
}

bool IntegerFilter::accept(const std::wstring& text, int& out) {
    if (text.empty()) {
        return false;
    }
    wchar_t* end = nullptr;
    const long v = std::wcstol(text.c_str(), &end, 10);
    if (!end || *end != L'\0') {
        return false;
    }
    out = static_cast<int>(v);
    return true;
}

bool UnsignedFloatFilter::accept(const std::wstring& text, float& out) {
    if (!FloatFilter::accept(text, out)) {
        return false;
    }
    return out >= 0.f;
}

bool UnsignedIntegerFilter::accept(const std::wstring& text, unsigned& out) {
    int v = 0;
    if (!IntegerFilter::accept(text, v) || v < 0) {
        return false;
    }
    out = static_cast<unsigned>(v);
    return true;
}

} // namespace plasma
