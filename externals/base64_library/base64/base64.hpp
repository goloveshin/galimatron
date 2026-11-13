#pragma once

// std
#include <string>
#include <string_view>
#include <vector>


// Google: c++ base64 header only library

namespace base64
{

inline constexpr std::string_view lookup =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string encode(std::string_view in)
{
    std::string out;
    out.reserve((in.size() + 2) / 3 * 4);
    int val = 0, valb = -6;
    for (unsigned char c : in)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back(lookup[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(lookup[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

inline std::string decode(std::string_view in)
{
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[lookup[i]] = i;

    std::string out;
    int val = 0, valb = -8;
    for (unsigned char c : in)
    {
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        while (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

}// namespace base64

// just mark string as base64-string
//
inline std::string_view operator""_base64(const char* str, const std::size_t len)
{
    return std::string_view(str, len);
}
