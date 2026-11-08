#include "court_connect.h"
#include "input.h"
#include "timing.h"
#include "httplib.h"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static const int RESULTS_SHOWN = 25;
static const int TIMING_RUNS = 5;

static std::vector<Player> all_players;
static std::string json_string(const std::string& text) { // json_string escapes a string for JSON, including quotes around it. rules from https://www.json.org/json-en.html
    std::string out = "\"";
    for (char c : text) { 
        if (c == '"') {
            out += "\\\"";
        } else if (c == '\\') {
            out += "\\\\";
        } else if (c == '\n') {
            out += "\\n";
        } else if (c == '\t') {
            out += "\\t";
        } else if (c == '\r') {} else {
            out += c;
        }
    }
    out += "\"";
    return out;
}
static std::string json_number(double value, int decimals) {
    if (!std::isfinite(value)) {
        return "null";
    }
    std::ostringstream out; // buffer the number into a string so we can control the decimal precision https://en.cppreference.com/cpp/io/basic_ostringstream 
    out << std::fixed << std::setprecision(decimals) << value; // fixed means always show the decimal point, even if the number is a whole number https://en.cppreference.com/w/cpp/io/manip/fixed 
    return out.str();
}

static std::string json_bool(bool value) {
    if (value) {
        return "true";
    }
    return "false";
}

static std::string error_json(const std::string& message) {
    return "{\"error\":" + json_string(message) + "}";
}

static std::string timing_json(const SortTiming& timing) {
    std::ostringstream out;
    out << "{\"avgMs\":" << json_number(timing.avgMs, 4) << ",\"minMs\":" << json_number(timing.minMs, 4)
        << ",\"maxMs\":" << json_number(timing.maxMs, 4) << ",\"runs\":" << timing.runs << "}";
    return out.str();
}

static double ms_since(std::chrono::steady_clock::time_point start) { // returns the number of milliseconds since the given time point, as a double. https://en.cppreference.com/w/cpp/chrono/steady_clock
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - start).count();// duration<double, std::milli> means a duration in milliseconds, stored as a double. https://en.cppreference.com/w/cpp/chrono/duration
}

static std::string get_param(const httplib::Request& req, const std::string& name) { // returns the value of a query parameter, or "" if it is not present. has_param checks first so the missing case is clear https://github.com/yhirose/cpp-httplib
    if (!req.has_param(name)) {
        return "";
    }
    return req.get_param_value(name);
}
static std::string read_query(const httplib::Request& req, SearchQuery& query) {
    query.skillLevel = parse_int(get_param(req, "skill"), 1, 10);
    if (query.skillLevel == INVALID_INT) {
        return "Skill level must be a whole number from 1 to 10.";
    }
    query.latitude = parse_double(get_param(req, "lat"), 24.0, 31.5);
    query.longitude = parse_double(get_param(req, "lon"), -88.0, -79.5);
    if (std::isnan(query.latitude) || std::isnan(query.longitude)) { // isnan returns true for NaN, false for any other number, including infinity https://en.cppreference.com/w/cpp/numeric/math/isnan
        return "Location must be inside Florida (latitude 24.0-31.5, longitude -88.0 to -79.5).";
    }
    query.day = parse_int(get_param(req, "day"), 0, 6);
    if (query.day == INVALID_INT) {
        return "Pick a day of the week.";
    }
    query.startMin = parse_time(get_param(req, "start"));
    query.endMin = parse_time(get_param(req, "end"));
    if (query.startMin == INVALID_TIME || query.endMin == INVALID_TIME) {
        return "Enter times like 6:30 PM, 6pm or 18:30.";
    }
    if (query.endMin <= query.startMin && query.startMin >= 12 * 60 && query.endMin < 12 * 60) {
        query.endMin += 12 * 60;
    }
    if (query.endMin <= query.startMin) {
        return "You entered " + format_time(query.startMin) + " to " + format_time(query.endMin) +
               ". Your end time must be after your start time.";
    }
    if (query.endMin - query.startMin < 60) {
        return "You entered " + format_time(query.startMin) + " to " + format_time(query.endMin) +
               ". Your window must be at least 60 minutes long.";
    }
    query.practiceType = get_param(req, "practice"); 
    bool known_practice = false;
    for (size_t i = 0; i < PRACTICE_TYPES.size(); i++) {
        if (PRACTICE_TYPES[i] == query.practiceType) {
            known_practice = true;
        }
    }
    if (!known_practice) {
        return "Pick a practice activity.";
    }

    query.maxDistanceMiles = parse_double(get_param(req, "dist"), 1, 500);
    if (std::isnan(query.maxDistanceMiles)) {
        return "Max travel distance must be from 1 to 500 miles.";
    }
    return "";
}

static void handle_meta(const httplib::Request& req, httplib::Response& res) { // handle_meta returns a JSON object with the number of players, the list of Florida cities, and the list of practice types. This is used by the web demo to populate the search form.
    std::ostringstream out;
    out << "{\"playerCount\":" << all_players.size() << ",\"cities\":[";
    for (size_t i = 0; i < FLORIDA_CITIES.size(); i++) {
        if (i > 0) {
            out << ",";
        }
        out << "{\"name\":" << json_string(FLORIDA_CITIES[i].name)
            << ",\"lat\":" << json_number(FLORIDA_CITIES[i].latitude, 4)
            << ",\"lon\":" << json_number(FLORIDA_CITIES[i].longitude, 4) << "}";
    }
    out << "],\"practiceTypes\":[";
    for (size_t i = 0; i < PRACTICE_TYPES.size(); i++) {
        if (i > 0) {
            out << ",";
        }
        out << json_string(PRACTICE_TYPES[i]);
    }
    out << "]}";
    res.set_content(out.str(), "application/json");
}

static void handle_search(const httplib::Request& req, httplib::Response& res) {
    SearchQuery query;
    std::string problem = read_query(req, query);
    if (problem != "") {
        res.status = 400;
        res.set_content(error_json(problem), "application/json");
        return;
    }
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::vector<Candidate> candidates = filter_players(all_players, query);
    double filter_ms = ms_since(start);

    start = std::chrono::steady_clock::now();
    score_candidates(candidates, query);
    double score_ms = ms_since(start);
     ComparisonResult comparison = compare_sorts(candidates, InputOrder::Random, TIMING_RUNS);
    std::vector<Candidate> ranked = candidates;
    merge_sort(ranked);
    std::ostringstream out;
        out << "{\"query\":{\"skill\":" << query.skillLevel << ",\"lat\":" << json_number(query.latitude, 4)
        << ",\"lon\":" << json_number(query.longitude, 4) << ",\"day\":" << query.day
        << ",\"dayName\":" << json_string(day_name(query.day)) << ",\"startMin\":" << query.startMin
        << ",\"endMin\":" << query.endMin << ",\"start\":" << json_string(format_time(query.startMin))
        << ",\"end\":" << json_string(format_time(query.endMin))
        << ",\"practice\":" << json_string(query.practiceType)
        << ",\"dist\":" << json_number(query.maxDistanceMiles, 1) << "}";
        out << ",\"total\":" << ranked.size() << ",\"searched\":" << all_players.size();
        out << ",\"timing\":{\"filterMs\":" << json_number(filter_ms, 4) << ",\"scoreMs\":" << json_number(score_ms, 4)
        << ",\"merge\":" << timing_json(comparison.merge) << ",\"heap\":" << timing_json(comparison.heap)
        << ",\"rankingsMatch\":" << json_bool(comparison.rankingsMatch) << "}";
        size_t shown_count = ranked.size();
    if (shown_count > (size_t)RESULTS_SHOWN) {
        shown_count = (size_t)RESULTS_SHOWN;
    }

    out << ",\"matches\":[";
    for (size_t i = 0; i < shown_count; i++) {
        const Candidate& candidate = ranked[i];
        if (i > 0) {
            out << ",";
        }
        out << "{\"rank\":" << i + 1 << ",\"id\":" << candidate.player.playerID
            << ",\"skill\":" << candidate.player.skillLevel
            << ",\"lat\":" << json_number(candidate.player.latitude, 4)
            << ",\"lon\":" << json_number(candidate.player.longitude, 4)
            << ",\"startMin\":" << candidate.player.startMin << ",\"endMin\":" << candidate.player.endMin
            << ",\"start\":" << json_string(format_time(candidate.player.startMin))
            << ",\"end\":" << json_string(format_time(candidate.player.endMin))
            << ",\"distance\":" << json_number(candidate.distanceMiles, 2)
            << ",\"overlap\":" << candidate.overlapMinutes
            << ",\"score\":" << json_number(candidate.score, 4) << "}";
    }
    out << "]}";
    res.set_content(out.str(), "application/json");
}

static void handle_benchmark(const httplib::Request& req, httplib::Response& res) {
    SearchQuery query;
    query.skillLevel = 5;
    query.latitude = FLORIDA_CITIES[0].latitude;
    query.longitude = FLORIDA_CITIES[0].longitude;

    std::vector<size_t> sizes = {1000, 10000, 50000, all_players.size()};
    std::vector<InputOrder> orders = {InputOrder::Random, InputOrder::AlreadySorted,
                                      InputOrder::ReverseSorted, InputOrder::AllTied};

    std::ostringstream out;
    out << "{\"runs\":" << TIMING_RUNS << ",\"results\":[";
    bool first_result = true;
    for (size_t n : sizes) {
        if (n == 0 || n > all_players.size()) {
            continue;
        }
        for (InputOrder order : orders) {
            std::vector<Candidate> input = make_benchmark_input(all_players, query, n, order);
            ComparisonResult result = compare_sorts(input, order, TIMING_RUNS);
            if (!first_result) {
                out << ",";
            }
            first_result = false;
            out << "{\"count\":" << result.candidateCount << ",\"order\":" << json_string(order_name(order))
                << ",\"merge\":" << timing_json(result.merge) << ",\"heap\":" << timing_json(result.heap)
                << ",\"rankingsMatch\":" << json_bool(result.rankingsMatch) << "}";
        }
    }
    out << "]}";
    res.set_content(out.str(), "application/json");
}

int main(int argc, char* argv[]) {
    std::string csv_path = "data/players.csv";
    if (argc > 1) {
        csv_path = argv[1];
    }
    int port = 8080;
    if (argc > 2) {
        port = parse_int(argv[2], 1, 65535);
        if (port == INVALID_INT) {
            std::cerr << "Port must be a number from 1 to 65535\n";
            return 1;
        }
    }
    all_players = load_players(csv_path);
    if (all_players.empty()) {
        std::cerr << "Could not load any players from " << csv_path << "\n";
        return 1;
    }
    httplib::Server server; // server setup follows the cpp-httplib README https://github.com/yhirose/cpp-httplib
    if (!server.set_mount_point("/", "./web")) {
        std::cerr << "Could not find the web/ folder. Run this from the Court-Connect folder.\n";
        return 1;
    }
    server.Get("/api/meta", handle_meta);
    server.Get("/api/search", handle_search);
    server.Get("/api/benchmark", handle_benchmark);
    std::cout << "Court Connect web demo\n"
              << "Loaded " << all_players.size() << " player profiles from " << csv_path << "\n"
              << "Open http://localhost:" << port << " in your browser (Ctrl+C to stop)\n";
    if (!server.listen("0.0.0.0", port)) {
        std::cerr << "Could not start the server on port " << port << ". Is it already in use?\n";
        return 1;
    }
    return 0;
}