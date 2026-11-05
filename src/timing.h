//timing.h
//OWNER: Zachary Martus
//times merge sort vs heap sort on the exact same copies of the candidate list
//only the sort call itself gets timed, everything else (copying, loading, filtering, scoring) happens off the clock

#pragma once

#include "court_connect.h"

#include <string>
#include <vector>

using SortFn = void (*)(std::vector<Candidate>&);

struct SortTiming {
    double avgMs = 0.0;
    double minMs = 0.0;
    double maxMs = 0.0;
    int runs = 0;
};

//the different ways we can arrange the input before sorting it
enum class InputOrder { Random, AlreadySorted, ReverseSorted, AllTied };

struct ComparisonResult {
    size_t candidateCount = 0;
    InputOrder order = InputOrder::Random;
    SortTiming merge;
    SortTiming heap;
    bool rankingsMatch = false;
};

//sorts a new copy of the input each run so every run starts from the same list
SortTiming time_sort(SortFn sort, const std::vector<Candidate>& input, int runs);

//checks that both lists have the same players in the same order
bool same_ranking(const std::vector<Candidate>& first, const std::vector<Candidate>& second);

//runs both sorts on the same input and makes sure they agree
ComparisonResult compare_sorts(const std::vector<Candidate>& input, InputOrder order, int runs);

//builds a scored list from the first player_count players with no filtering, then arranges it in the given order
std::vector<Candidate> make_benchmark_input(const std::vector<Player>& players, const SearchQuery& query,
                                            size_t player_count, InputOrder order);

std::string order_name(InputOrder order);