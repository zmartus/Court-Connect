//court_connect.h
//shared types and function names, everyone codes against these

#pragma once

#include <string>
#include <vector>

struct Player {
    int playerID = 0;
    //skill goes from 1 to 10
    int skillLevel = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    //0 is Sunday through 6 for Saturday
    int availableDay = 0;
    //start and end are both stored as minutes after midnight
    int startMin = 0;
    int endMin = 0;
    std::string practiceType;
};

inline const std::vector<std::string> PRACTICE_TYPES =
    {"shooting", "defense", "one_on_one", "conditioning"};

struct City {
    std::string name;
    double latitude;
    double longitude;
};
inline const std::vector<City> FLORIDA_CITIES = {
    {"Gainesville", 29.6516, -82.3248},
    {"Jacksonville", 30.3322, -81.6557},
    {"Orlando", 28.5384, -81.3789},
    {"Tampa", 27.9506, -82.4572},
    {"Miami", 25.7617, -80.1918},
    {"Tallahassee", 30.4383, -84.2807},
};

struct SearchQuery {
    int skillLevel = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    int day = 0;
    int startMin = 0;
    int endMin = 0;
    std::string practiceType;
    double maxDistanceMiles = 0.0;
};

struct Candidate {
    Player player;
    double distanceMiles = 0.0;
    int overlapMinutes = 0;
    //a lower score means a better match
    double score = 0.0;
};

//both sorts use this: lower score first, then lower playerID
inline bool candidate_less(const Candidate& first, const Candidate& second) {
    if (first.score != second.score) {
        return first.score < second.score;
    }
    return first.player.playerID < second.player.playerID;
}

//Zachery (src/geo.cpp)
double haversine_miles(double from_latitude, double from_longitude, double to_latitude, double to_longitude);

//Shrish (src/data_loader.cpp, src/merge_sort.cpp)
std::vector<Player> load_players(const std::string& csv_path);
void merge_sort(std::vector<Candidate>& candidates);

//Zachery (src/filter.cpp, src/heap_sort.cpp)
std::vector<Candidate> filter_players(const std::vector<Player>& players, const SearchQuery& query);
void score_candidates(std::vector<Candidate>& candidates, const SearchQuery& query);
void heap_sort(std::vector<Candidate>& candidates);
