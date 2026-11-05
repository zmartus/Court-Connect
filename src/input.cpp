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
    //lowercase copy so "PM", "pm" and "Pm" all work the same
    std::string time_text = trim(text);
    for (size_t i = 0; i < time_text.size(); i++) {
        time_text[i] = (char)std::tolower((unsigned char)time_text[i]);
    }

    //if it ends in am or pm we read it as 12-hour time, otherwise 24-hour
    bool twelve_hour = false;
    bool is_pm = false;
    if (time_text.size() >= 2) {
        std::string ending = time_text.substr(time_text.size() - 2);
        if (ending == "am" || ending == "pm") {
            twelve_hour = true;
            if (ending == "pm") {
                is_pm = true;
            }
            time_text = trim(time_text.substr(0, time_text.size() - 2));
        }
    }

    int max_hour = 23;
    int min_hour = 0;
    if (twelve_hour) {
        max_hour = 12;
        min_hour = 1;
    }

    int hours = INVALID_INT;
    int minutes = 0;
    size_t colon = time_text.find(':');
    if (colon == std::string::npos) {
        //no colon is only ok with am/pm, like "9pm"
        if (!twelve_hour) {
            return INVALID_TIME;
        }
        hours = parse_int(time_text, min_hour, max_hour);
    } else {
        //there has to be an hour before the colon and exactly two digits after it
        if (colon == 0 || colon + 3 != time_text.size()) {
            return INVALID_TIME;
        }
        hours = parse_int(time_text.substr(0, colon), min_hour, max_hour);
        minutes = parse_int(time_text.substr(colon + 1), 0, 59);
    }
    if (hours == INVALID_INT || minutes == INVALID_INT) {
        return INVALID_TIME;
    }

    if (twelve_hour) {
        //12 am is midnight and 12 pm is noon, so 12 goes back to 0 before adding the pm hours
        if (hours == 12) {
            hours = 0;
        }
        if (is_pm) {
            hours = hours + 12;
        }
    }
    return hours * 60 + minutes;
}

std::string format_time(int minutes) {
    int hours = (minutes / 60) % 24;
    std::string suffix = "AM";
    if (hours >= 12) {
        suffix = "PM";
    }
    //a 12-hour clock goes 12, 1, 2 ... 11, so both 0 and 12 show up as 12
    int clock_hour = hours % 12;
    if (clock_hour == 0) {
        clock_hour = 12;
    }
    //9 chars is enough for "12:00 PM" plus the null at the end
    char buffer[9];
    std::snprintf(buffer, sizeof(buffer), "%d:%02d %s", clock_hour, minutes % 60, suffix.c_str());
    return buffer;
}

std::string day_name(int day) {
    static const char* names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    if (day < 0 || day > 6) {
        return "?";
    }
    return names[day];
}