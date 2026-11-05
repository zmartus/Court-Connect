// OWNER: Zachery Lowry
#include "court_connect.h"
#include <algorithm>
#include <cmath>

std::vector<Candidate> filter_players(const std::vector<Player>& players,
                                      const SearchQuery& query) {
    std::vector<Candidate> matches;
    // Avoid invalid search windows or distance limits.
    if (!std::isfinite(query.maxDistanceMiles) || query.maxDistanceMiles <= 0.0 ||
        query.endMin - query.startMin < 60 ||
        query.day < 0 || query.day > 6) {
        return matches;
    }

    for (const Player& player : players) {
        if (player.availableDay != query.day ||
            player.practiceType != query.practiceType) {
            continue;
        }

        const int overlap = std::min(player.endMin, query.endMin) -
                            std::max(player.startMin, query.startMin);
        if (overlap < 60) {
            continue;
        }

        const double distance = haversine_miles(query.latitude, query.longitude,
                                                player.latitude, player.longitude);
        if (!std::isfinite(distance) || distance > query.maxDistanceMiles) {
            continue;
        }

        Candidate candidate;
        candidate.player = player;
        candidate.distanceMiles = distance;
        candidate.overlapMinutes = overlap;
        matches.push_back(candidate);
    }
    return matches;
}

void score_candidates(std::vector<Candidate>& candidates, const SearchQuery& query) {
    // Skill difference is 0..9; normalize to 0..1.
    // Distance is normalized by the user's search radius (or benchmark fallback).
    // A lower score means a better match.
    const double radius = (std::isfinite(query.maxDistanceMiles) &&
                           query.maxDistanceMiles > 0.0)
                              ? query.maxDistanceMiles : 500.0;

    for (Candidate& candidate : candidates) {
        const double skill_penalty =
            std::abs(candidate.player.skillLevel - query.skillLevel) / 9.0;
        const double distance_penalty = candidate.distanceMiles / radius;
        candidate.score = 0.70 * skill_penalty + 0.30 * distance_penalty;
    }
}
