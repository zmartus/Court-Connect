//input.cpp
//OWNER: Zachary Martus

#include "input.h"

#include <cctype>
#include <cmath>
#include <cstdio>

std::string trim(const std::string& text) {
    //walk forward past any spaces at the front
    size_t start = 0;
    while (start < text.size() && std::isspace((unsigned char)text[start])) {
        start++;
    }
    //then walk backward past any spaces at the end
    size_t end = text.size();
    while (end > start && std::isspace((unsigned char)text[end - 1])) {
        end--;
    }
    return text.substr(start, end - start);
}

int parse_int(const std::string& text, int min_value, int max_value) {
    std::string trimmed = trim(text);
    if (trimmed.empty()) {
        return INVALID_INT;
    }
    try {
        size_t used = 0;
        long value = std::stol(trimmed, &used);
        //if stol didn't use every character there was junk after the number, like "7abc"
        if (used != trimmed.size() || value < min_value || value > max_value) {
            return INVALID_INT;
        }
        return (int)value;
    } catch (...) {
        return INVALID_INT;
    }
}

double parse_double(const std::string& text, double min_value, double max_value) {
    //nan means bad input, it can never be a real answer since we reject nan below anyway
    double invalid = std::nan("");
    std::string trimmed = trim(text);
    if (trimmed.empty()) {
        return invalid;
    }
    try {
        size_t used = 0;
        double value = std::stod(trimmed, &used);
        //also reject inf and nan since stod will happily parse those
        if (used != trimmed.size() || !std::isfinite(value) || value < min_value || value > max_value) {
            return invalid;
        }
        return value;
    } catch (...) {
        return invalid;
    }
}

int parse_time(const std::string& text) {
    std::string trimmed = trim(text);
    size_t colon = trimmed.find(':');
    //there has to be an hour before the colon and exactly two digits after it
    if (colon == std::string::npos || colon == 0 || colon + 3 != trimmed.size()) {
        return INVALID_TIME;
    }
    int hours = parse_int(trimmed.substr(0, colon), 0, 23);
    int minutes = parse_int(trimmed.substr(colon + 1), 0, 59);
    if (hours == INVALID_INT || minutes == INVALID_INT) {
        return INVALID_TIME;
    }
    return hours * 60 + minutes;
}

std::string format_time(int minutes) {
    //6 chars is enough for "HH:MM" plus the null at the end
    char buffer[6];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", (minutes / 60) % 24, minutes % 60);
    return buffer;
}

std::string day_name(int day) {
    static const char* names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    if (day < 0 || day > 6) {
        return "?";
    }
    return names[day];
}