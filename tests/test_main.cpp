//test_main.cpp
//small self-contained checks, run them with ./court_connect_tests

#include "court_connect.h"
#include "input.h"
#include "menu.h"
#include "timing.h"

#include <cmath>
#include <iostream>
#include <sstream>

static int failures = 0;

//prints the file, line and condition that failed instead of stopping, so we see every failure in one run
#define CHECK(cond)                                                              \
    do {                                                                         \
        if (!(cond)) {                                                           \
            std::cerr << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n"; \
            failures++;                                                          \
        }                                                                        \
    } while (0)

static std::vector<Player> sample_players() {
    //three players near Gainesville on Monday evening, one far away, and one on the wrong day
    std::vector<Player> players(5);
    players[0] = {1, 5, 29.65, -82.32, 1, 17 * 60, 19 * 60, "shooting"};
    players[1] = {2, 7, 29.66, -82.33, 1, 18 * 60, 20 * 60, "shooting"};
    players[2] = {3, 5, 29.70, -82.40, 1, 17 * 60, 21 * 60, "shooting"};
    //this one is in Miami so it should be too far
    players[3] = {4, 5, 25.76, -80.19, 1, 17 * 60, 19 * 60, "shooting"};
    //this one is free on Wednesday instead of Monday
    players[4] = {5, 5, 29.65, -82.32, 3, 17 * 60, 19 * 60, "shooting"};
    return players;
}

static void test_parsing() {
    CHECK(parse_int("7", 1, 10) == 7);
    CHECK(parse_int(" 10 ", 1, 10) == 10);
    CHECK(parse_int("11", 1, 10) == INVALID_INT);
    CHECK(parse_int("7abc", 1, 10) == INVALID_INT);
    CHECK(parse_int("", 1, 10) == INVALID_INT);
    CHECK(parse_double("12.5", 1, 500) == 12.5);
    CHECK(std::isnan(parse_double("nan", 1, 500)));
    CHECK(std::isnan(parse_double("-3", 1, 500)));
    CHECK(parse_time("18:30") == 18 * 60 + 30);
    CHECK(parse_time("6:05") == 6 * 60 + 5);
    CHECK(parse_time("24:00") == INVALID_TIME);
    CHECK(parse_time("18:3") == INVALID_TIME);
    CHECK(parse_time("1830") == INVALID_TIME);
    CHECK(format_time(18 * 60 + 5) == "18:05");
    CHECK(day_name(0) == "Sun");
}

static void test_timing() {
    //make 2000 fake players near Gainesville so the benchmark has something to sort
    std::vector<Player> players;
    for (int i = 1; i <= 2000; i++) {
        Player player;
        player.playerID = i;
        player.skillLevel = (i % 10) + 1;
        player.latitude = 29.6 + (i % 50) * 0.01;
        player.longitude = -82.3 - (i % 30) * 0.01;
        player.practiceType = "shooting";
        players.push_back(player);
    }
    SearchQuery query;
    query.skillLevel = 5;
    query.latitude = FLORIDA_CITIES[0].latitude;
    query.longitude = FLORIDA_CITIES[0].longitude;

    for (InputOrder order : {InputOrder::Random, InputOrder::AlreadySorted, InputOrder::ReverseSorted,
                             InputOrder::AllTied}) {
        std::vector<Candidate> input = make_benchmark_input(players, query, 2000, order);
        CHECK(input.size() == 2000);
        ComparisonResult result = compare_sorts(input, order, 2);
        CHECK(result.rankingsMatch);
        CHECK(result.merge.runs == 2 && result.heap.runs == 2);
        CHECK(result.merge.minMs <= result.merge.avgMs && result.merge.avgMs <= result.merge.maxMs);
    }

    //same two players but in opposite order, so they shouldn't count as the same ranking
    std::vector<Candidate> first(2);
    std::vector<Candidate> second(2);
    first[0].player.playerID = 1;
    first[1].player.playerID = 2;
    second[0].player.playerID = 2;
    second[1].player.playerID = 1;
    CHECK(!same_ranking(first, second));
    CHECK(same_ranking(first, first));
}

static void test_menu_search() {
    //find partners with skill 5, Gainesville, Monday, 17:00-19:00, shooting, 20 miles, then exit
    std::istringstream in("1\n5\n1\n1\n17:00\n19:00\n1\n20\n3\n");
    std::ostringstream out;
    run_app(sample_players(), in, out);
    std::string output = out.str();
    CHECK(output.find("3 players matched") != std::string::npos);
    CHECK(output.find("Goodbye!") != std::string::npos);
}

static void test_menu_invalid_input_and_no_matches() {
    //bad menu choice, bad skill, a window that's too short, then a Saturday search nobody matches
    std::istringstream in("9\n1\nabc\n5\n1\n6\n17:00\n17:30\n17:00\n19:00\n1\n20\n3\n");
    std::ostringstream out;
    run_app(sample_players(), in, out);
    std::string output = out.str();
    CHECK(output.find("Please enter a whole number from 1 to 3") != std::string::npos);
    CHECK(output.find("Please enter a whole number from 1 to 10") != std::string::npos);
    CHECK(output.find("at least 60 minutes") != std::string::npos);
    CHECK(output.find("No players matched") != std::string::npos);
}

static void test_menu_input_closed() {
    //input stops in the middle of the search form, so the menu should catch it and exit
    std::istringstream in("1\n5\n");
    std::ostringstream out;
    run_app(sample_players(), in, out);
    CHECK(out.str().find("Input closed") != std::string::npos);
}

int main() {
    test_parsing();
    test_timing();
    test_menu_search();
    test_menu_invalid_input_and_no_matches();
    test_menu_input_closed();
    if (failures == 0) {
        std::cout << "All tests passed.\n";
        return 0;
    }
    return 1;
}
 