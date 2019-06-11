#pragma once

#include <string>
#include <algorithm>
#include <cctype>
#include <locale>

/**
 * Trim from start (in place)
 */
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

/**
 * Trim from end (in place)
 */
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

/**
 * Trim from both ends (in place)
 */
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

/**
 * Trim from start (copying)
 */
static inline std::string ltrim_copy(std::string s) {
    ltrim(s);
    return s;
}

/**
 * Trim from end (copying)
 */
static inline std::string rtrim_copy(std::string s) {
    rtrim(s);
    return s;
}

/**
 * Trim from both ends (copying)
 */
static inline std::string trim_copy(std::string s) {
    trim(s);
    return s;
}
