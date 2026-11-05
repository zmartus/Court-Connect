//input.h
//OWNER: Zachary Martus
//helpers for reading and printing stuff in the terminal menu
//every parser gives back a special "invalid" value if the input is bad, so the menu can just ask again

#pragma once

#include <climits>
#include <string>

//parse_int gives this back when the input is bad
const int INVALID_INT = INT_MIN;

//parse_time gives this back when the input is bad, real times are never negative
const int INVALID_TIME = -1;

std::string trim(const std::string& text);

//the whole string has to be a number in range, so "7" works but "7abc" or "" don't
int parse_int(const std::string& text, int min_value, int max_value);

//same idea as parse_int but for decimals, gives back nan when the input is bad (check with std::isnan)
double parse_double(const std::string& text, double min_value, double max_value);

//turns "18:30" or "6:05" into minutes after midnight (hours 0-23, minutes 0-59)
int parse_time(const std::string& text);

//goes the other way, so 1110 becomes "18:30"
std::string format_time(int minutes);

//0 is "Sun", 1 is "Mon", and so on
std::string day_name(int day);