//merge_sort.cpp
//OWNER: Shrish Arun

#include "court_connect.h"
#include <vector>
using namespace std;
void merge_halves(vector<Candidate>& candidates, vector<Candidate>& temp, int left, int middle, int right) {
    int left_index = left;
    int right_index = middle + 1;
    int temp_index = left;

    while (left_index <= middle && right_index <= right) {
        if (candidate_less(candidates[right_index], candidates[left_index])) {
            temp[temp_index] = candidates[right_index];
            right_index++;
        } else {
            temp[temp_index] = candidates[left_index];
            left_index++;
        }
        temp_index++;
    }
    while (left_index <= middle) {
        temp[temp_index] = candidates[left_index];
        left_index++;
        temp_index++;
    }
    while (right_index <= right) {
        temp[temp_index] = candidates[right_index];
        right_index++;
        temp_index++;
    }
    for (int i = left; i <= right; i++) {
        candidates[i] = temp[i];
    }
}
void merge_sort_helper(vector<Candidate>& candidates, vector<Candidate>& temp, int left, int right) {
    if (left >= right) {
        return;
    }
    int middle = left + (right - left) / 2;
    merge_sort_helper(candidates, temp, left, middle);
    merge_sort_helper(candidates, temp, middle + 1, right);
    merge_halves(candidates, temp, left, middle, right);
}
void merge_sort(vector<Candidate>& candidates) {
    if(candidates.size() < 2) {
        return;
    }
    vector<Candidate> temp(candidates.size());
    merge_sort_helper(candidates, temp, 0, candidates.size() - 1);
}