//timing.cpp
//OWNER: Zachary Martus

#include "timing.h"

#include <algorithm>
#include <chrono>
#include <random>

SortTiming time_sort(SortFn sort, const std::vector<Candidate>& input, int runs) {
    SortTiming timing;
    timing.runs = runs;
    if (timing.runs < 1) {
        timing.runs = 1;
    }

    double total = 0.0;
    for (int i = 0; i < timing.runs; i++) {
        //make the copy before starting the clock so copying doesn't count toward the sort time
        std::vector<Candidate> copy = input;

        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        sort(copy);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
        total += elapsed_ms;
        if (i == 0 || elapsed_ms < timing.minMs) {
            timing.minMs = elapsed_ms;
        }
        if (i == 0 || elapsed_ms > timing.maxMs) {
            timing.maxMs = elapsed_ms;
        }
    }
    timing.avgMs = total / timing.runs;
    return timing;
}

bool same_ranking(const std::vector<Candidate>& first, const std::vector<Candidate>& second) {
    if (first.size() != second.size()) {
        return false;
    }
    for (size_t i = 0; i < first.size(); i++) {
        if (first[i].player.playerID != second[i].player.playerID) {
            return false;
        }
    }
    return true;
}

ComparisonResult compare_sorts(const std::vector<Candidate>& input, InputOrder order, int runs) {
    ComparisonResult result;
    result.candidateCount = input.size();
    result.order = order;
    result.merge = time_sort(merge_sort, input, runs);
    result.heap = time_sort(heap_sort, input, runs);

    //run each sort one more time (not timed) just to make sure both give the same ranking
    std::vector<Candidate> by_merge = input;
    std::vector<Candidate> by_heap = input;
    merge_sort(by_merge);
    heap_sort(by_heap);
    result.rankingsMatch = same_ranking(by_merge, by_heap);
    return result;
}

std::vector<Candidate> make_benchmark_input(const std::vector<Player>& players, const SearchQuery& query,
                                            size_t player_count, InputOrder order) {
    //can't use more players than we actually have
    if (player_count > players.size()) {
        player_count = players.size();
    }

    std::vector<Candidate> candidates;
    candidates.reserve(player_count);
    for (size_t i = 0; i < player_count; i++) {
        Candidate candidate;
        candidate.player = players[i];
        candidate.distanceMiles = haversine_miles(query.latitude, query.longitude,
                                                 players[i].latitude, players[i].longitude);
        candidates.push_back(candidate);
    }

    SearchQuery bench_query = query;
    if (bench_query.maxDistanceMiles <= 0) {
        bench_query.maxDistanceMiles = 500.0;
    }
    score_candidates(candidates, bench_query);

    //using a fixed seed so the "random" order is the same every time we run the benchmark
    std::mt19937 generator(7);
    switch (order) {
        case InputOrder::Random:
            std::shuffle(candidates.begin(), candidates.end(), generator);
            break;
        case InputOrder::AlreadySorted:
            merge_sort(candidates);
            break;
        case InputOrder::ReverseSorted: {
            merge_sort(candidates);
            //flip the sorted list by swapping from both ends until we hit the middle
            size_t count = candidates.size();
            for (size_t i = 0; i < count / 2; i++) {
                Candidate temp = candidates[i];
                candidates[i] = candidates[count - 1 - i];
                candidates[count - 1 - i] = temp;
            }
            break;
        }
        case InputOrder::AllTied:
            //give everyone the same score so playerID is the only thing left to break ties
            for (Candidate& candidate : candidates) {
                candidate.score = 0.5;
            }
            std::shuffle(candidates.begin(), candidates.end(), generator);
            break;
    }
    return candidates;
}

std::string order_name(InputOrder order) {
    switch (order) {
        case InputOrder::Random:
            return "random";
        case InputOrder::AlreadySorted:
            return "already sorted";
        case InputOrder::ReverseSorted:
            return "reverse sorted";
        case InputOrder::AllTied:
            return "all tied";
    }
    return "";
}