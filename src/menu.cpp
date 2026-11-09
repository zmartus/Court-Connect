//menu.cpp
//OWNER: Zachary Martus

#include "menu.h"

#include "input.h"
#include "timing.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

static const int TOP_N = 10;
static const int TIMING_RUNS = 5;

//set to true when stdin closes, every screen checks it and backs out to run_app so we can exit cleanly
static bool input_closed = false;

static std::string ask_line(std::istream& in, std::ostream& out, const std::string& prompt) {
    out << prompt;
    std::string line;
    if (!std::getline(in, line)) {
        input_closed = true;
        return "";
    }
    return line;
}

static int ask_int(std::istream& in, std::ostream& out, const std::string& prompt, int min_value, int max_value) {
    while (true) {
        std::string line = ask_line(in, out, prompt);
        if (input_closed) {
            return INVALID_INT;
        }
        int value = parse_int(line, min_value, max_value);
        if (value != INVALID_INT) {
            return value;
        }
        out << "  Please enter a whole number from " << min_value << " to " << max_value << ".\n";
    }
}

static double ask_double(std::istream& in, std::ostream& out, const std::string& prompt, double min_value,
                         double max_value) {
    while (true) {
        std::string line = ask_line(in, out, prompt);
        if (input_closed) {
            return std::nan("");
        }
        double value = parse_double(line, min_value, max_value);
        if (!std::isnan(value)) {
            return value;
        }
        out << "  Please enter a number from " << min_value << " to " << max_value << ".\n";
    }
}

static int ask_time(std::istream& in, std::ostream& out, const std::string& prompt) {
    while (true) {
        std::string line = ask_line(in, out, prompt);
        if (input_closed) {
            return INVALID_TIME;
        }
        int value = parse_time(line);
        if (value != INVALID_TIME) {
            return value;
        }
        out << "  Please enter a time like 6:30 PM, 6pm or 18:30.\n";
    }
}

static void print_header(std::ostream& out, const std::string& title) {
    out << "\n==== " << title << " ====\n";
}

//if input closes partway through, this returns early and run_app checks input_closed
static SearchQuery search_form(std::istream& in, std::ostream& out) {
    print_header(out, "Search Form");
    SearchQuery query;

    query.skillLevel = ask_int(in, out, "Your skill level (1-10): ", 1, 10);
    if (input_closed) {
        return query;
    }

    out << "Where are you?\n";
    for (size_t i = 0; i < FLORIDA_CITIES.size(); i++) {
        out << "  " << i + 1 << ") " << FLORIDA_CITIES[i].name << "\n";
    }

    int other_option = (int)FLORIDA_CITIES.size() + 1;
    out << "  " << other_option << ") Enter coordinates\n";
    int city_choice = ask_int(in, out, "Choose: ", 1, other_option);
    if (input_closed) {
        return query;
    }
    if (city_choice == other_option) {
        query.latitude = ask_double(in, out, "Latitude (24.0-31.5): ", 24.0, 31.5);
        if (input_closed) {
            return query;
        }
        query.longitude = ask_double(in, out, "Longitude (-88.0 to -79.5): ", -88.0, -79.5);
        if (input_closed) {
            return query;
        }
    } else {
        query.latitude = FLORIDA_CITIES[city_choice - 1].latitude;
        query.longitude = FLORIDA_CITIES[city_choice - 1].longitude;
    }

    out << "Day you want to practice:\n";
    for (int day = 0; day <= 6; day++) {
        out << "  " << day << ") " << day_name(day) << "\n";
    }
    query.day = ask_int(in, out, "Choose: ", 0, 6);
    if (input_closed) {
        return query;
    }

    //keep asking until the window is at least an hour long and actually ends after it starts
    while (true) {
        query.startMin = ask_time(in, out, "Free from (like 5:00 PM): ");
        if (input_closed) {
            return query;
        }
        query.endMin = ask_time(in, out, "Free until (like 7:00 PM): ");
        if (input_closed) {
            return query;
        }
        if (query.endMin <= query.startMin && query.startMin >= 12 * 60 && query.endMin < 12 * 60) {
            query.endMin += 12 * 60;
        }
        if (query.endMin <= query.startMin) {
            out << "  You entered " << format_time(query.startMin) << " to " << format_time(query.endMin)
                << ". Your end time must be after your start time (add AM or PM to be sure).\n";
            continue;
        }
        if (query.endMin - query.startMin < 60) {
            out << "  You entered " << format_time(query.startMin) << " to " << format_time(query.endMin)
                << ". Your window must be at least 60 minutes long.\n";
            continue;
        }
        break;
    }

    out << "Practice activity:\n";
    for (size_t i = 0; i < PRACTICE_TYPES.size(); i++) {
        out << "  " << i + 1 << ") " << PRACTICE_TYPES[i] << "\n";
    }
    int practice_choice = ask_int(in, out, "Choose: ", 1, (int)PRACTICE_TYPES.size());
    if (input_closed) {
        return query;
    }
    query.practiceType = PRACTICE_TYPES[practice_choice - 1];

    query.maxDistanceMiles = ask_double(in, out, "Max travel distance in miles (1-500): ", 1, 500);
    return query;
}

static void show_results(std::ostream& out, const std::vector<Candidate>& ranked) {
    print_header(out, "Match Results");
    if (ranked.empty()) {
        out << "No players matched your search.\n"
            << "Try a larger travel distance, a longer time window, or another day.\n";
        return;
    }

    //only show the top few, or fewer if there aren't that many matches
    size_t shown_count = ranked.size();
    if (shown_count > (size_t)TOP_N) {
        shown_count = (size_t)TOP_N;
    }

    out << ranked.size() << " players matched. Top " << shown_count << ":\n\n";
    out << std::left << std::setw(6) << "Rank" << std::setw(10) << "Player" << std::setw(7) << "Skill"
        << std::setw(11) << "Distance" << std::setw(24) << "Available" << std::setw(10) << "Overlap"
        << "Score\n";
    out << std::string(74, '-') << "\n";

    for (size_t i = 0; i < shown_count; i++) {
        const Candidate& candidate = ranked[i];
        std::string available = day_name(candidate.player.availableDay) + " " +
                                format_time(candidate.player.startMin) + "-" +
                                format_time(candidate.player.endMin);
        std::ostringstream miles;
        miles << std::fixed << std::setprecision(1) << candidate.distanceMiles << " mi";

        out << std::left << std::setw(6) << i + 1 << std::setw(10) << candidate.player.playerID
            << std::setw(7) << candidate.player.skillLevel << std::setw(11) << miles.str()
            << std::setw(24) << available << std::setw(10)
            << (std::to_string(candidate.overlapMinutes) + " min") << std::fixed << std::setprecision(3)
            << candidate.score << "\n";
    }
    out << "\nLower score = better match (70% skill difference, 30% distance).\n";
}

static void print_comparison_table(std::ostream& out, const std::vector<ComparisonResult>& results) {
    out << std::left << std::setw(12) << "Candidates" << std::setw(17) << "Input order" << std::setw(14)
        << "Merge (ms)" << std::setw(14) << "Heap (ms)" << std::setw(8) << "Faster" << "Same ranking\n";
    out << std::string(77, '-') << "\n";
    for (const ComparisonResult& result : results) {
        //merge wins ties
        std::string faster = "heap";
        if (result.merge.avgMs <= result.heap.avgMs) {
            faster = "merge";
        }
        std::string same_ranking_text = "NO";
        if (result.rankingsMatch) {
            same_ranking_text = "yes";
        }

        out << std::left << std::setw(12) << result.candidateCount << std::setw(17)
            << order_name(result.order) << std::fixed << std::setprecision(3) << std::setw(14)
            << result.merge.avgMs << std::setw(14) << result.heap.avgMs << std::setw(8) << faster
            << same_ranking_text << "\n";
    }
    out << "\nTimes are the average of " << TIMING_RUNS
        << " runs and cover only the sort (not loading, filtering or scoring).\n";
}

static void compare_screen(std::istream& in, std::ostream& out, const std::vector<Player>& players,
                           bool has_last_query, const SearchQuery& last_query,
                           const std::vector<Candidate>& last_candidates) {
    print_header(out, "Algorithm Comparison");
    out << "  1) Compare on my last search's candidates\n"
        << "  2) Full benchmark (sizes up to all " << players.size() << " players, 4 input orders)\n"
        << "  3) Back\n";
    int choice = ask_int(in, out, "Choose: ", 1, 3);
    if (input_closed) {
        return;
    }

    if (choice == 1) {
        if (!has_last_query) {
            out << "Run a search first (Main Menu option 1).\n";
            return;
        }
        if (last_candidates.empty()) {
            out << "Your last search had no candidates, so there is nothing to sort.\n";
            return;
        }
        out << "\nCandidate count: " << last_candidates.size() << "\n\n";
        std::vector<ComparisonResult> single_result;
        single_result.push_back(compare_sorts(last_candidates, InputOrder::Random, TIMING_RUNS));
        print_comparison_table(out, single_result);
    } else if (choice == 2) {
        //the benchmark scores everyone against the last search's location and skill
        SearchQuery query;
        if (has_last_query) {
            query = last_query;
        } else {
            query.skillLevel = 5;
            query.latitude = FLORIDA_CITIES[0].latitude;
            query.longitude = FLORIDA_CITIES[0].longitude;
        }

        std::vector<size_t> sizes = {1000, 10000, 50000, players.size()};
        std::vector<InputOrder> orders = {InputOrder::Random, InputOrder::AlreadySorted,
                                          InputOrder::ReverseSorted, InputOrder::AllTied};
        out << "\nRunning benchmark...\n\n";
        std::vector<ComparisonResult> results;
        for (size_t n : sizes) {
            //skip any size we don't actually have enough players for
            if (n == 0 || n > players.size()) {
                continue;
            }
            for (InputOrder order : orders) {
                std::vector<Candidate> input = make_benchmark_input(players, query, n, order);
                results.push_back(compare_sorts(input, order, TIMING_RUNS));
            }
        }
        print_comparison_table(out, results);
    }
}

void run_app(const std::vector<Player>& players, std::istream& in, std::ostream& out) {
    //reset in case run_app gets called more than once, like in the tests
    input_closed = false;
    bool has_last_query = false;
    SearchQuery last_query;
    std::vector<Candidate> last_candidates;

    out << "Court Connect: find a basketball practice partner\n"
        << "Loaded " << players.size() << " player profiles.\n";

    while (true) {
        print_header(out, "Main Menu");
        out << "  1) Find partners\n  2) Compare algorithms\n  3) Exit\n";
        int choice = ask_int(in, out, "Choose: ", 1, 3);
        if (input_closed) {
            break;
        }

        if (choice == 1) {
            SearchQuery query = search_form(in, out);
            if (input_closed) {
                break;
            }
            std::vector<Candidate> candidates = filter_players(players, query);
            score_candidates(candidates, query);
            has_last_query = true;
            last_query = query;
            //save the unsorted list so the comparison screen has something to sort later
            last_candidates = candidates;

            merge_sort(candidates);
            show_results(out, candidates);
        } else if (choice == 2) {
            compare_screen(in, out, players, has_last_query, last_query, last_candidates);
            if (input_closed) {
                break;
            }
        } else {
            out << "Goodbye!\n";
            return;
        }
    }

    //we only get here if the input ran out
    out << "\nInput closed. Goodbye!\n";
}