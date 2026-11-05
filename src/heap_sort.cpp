// OWNER: Zachery Lowry
#include "court_connect.h"
#include <algorithm>
#include <cstddef>

// Fix a max heap by pushing the smaller-priority parent downward.
// candidate_less defines "better" as smaller score, then smaller ID.
static void sift_down(std::vector<Candidate>& candidates,
                      std::size_t root, std::size_t count) {
    while (root < count / 2) {
        const std::size_t left = root * 2 + 1;
        std::size_t largest = root;
        if (candidate_less(candidates[largest], candidates[left])) {
            largest = left;
        }
        const std::size_t right = left + 1;
        if (right < count && candidate_less(candidates[largest], candidates[right])) {
            largest = right;
        }
        if (largest == root) return;
        std::swap(candidates[root], candidates[largest]);
        root = largest;
    }
}

void heap_sort(std::vector<Candidate>& candidates) {
    const std::size_t count = candidates.size();
    if (count < 2) return;

    // Bottom-up heap construction: O(n).
    for (std::size_t i = count / 2; i > 0; --i) {
        sift_down(candidates, i - 1, count);
    }

    // Move the greatest element to the end, then restore the heap.
    for (std::size_t end = count; end > 1; --end) {
        std::swap(candidates[0], candidates[end - 1]);
        sift_down(candidates, 0, end - 1);
    }
}
