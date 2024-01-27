#include "common.h"
// Tables below exist in jutils.c, but I dont want to waste my time looking for way to include them - copy-paste is faster
const int my_jpeg_zigzag_order[DCTSIZE2] = {
   0,  1,  5,  6, 14, 15, 27, 28,
   2,  4,  7, 13, 16, 26, 29, 42,
   3,  8, 12, 17, 25, 30, 41, 43,
   9, 11, 18, 24, 31, 40, 44, 53,
  10, 19, 23, 32, 39, 45, 52, 54,
  20, 22, 33, 38, 46, 51, 55, 60,
  21, 34, 37, 47, 50, 56, 59, 61,
  35, 36, 48, 49, 57, 58, 62, 63
};

const int my_jpeg_natural_order[DCTSIZE2] = {
   0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

void to_zigzag(const JCOEFPTR in) {
    JBLOCK buf;
    for (int i = 0; i < DCTSIZE2; ++i) {
        buf[my_jpeg_zigzag_order[i]] = in[i];
    }
    for (int i = 0; i < DCTSIZE2; ++i) {
        in[i] = buf[i];
    }
    return;
}

void from_zigzag(const JCOEFPTR in) {
    JBLOCK buf;
    for (int i = 0; i < DCTSIZE2; ++i) {
        buf[my_jpeg_natural_order[i]] = in[i];
    }
    for (int i = 0; i < DCTSIZE2; ++i) {
        in[i] = buf[i];
    }
    return;
}

JCOEF find_quant_step(const JCOEFPTR arr, const size_t begin, const size_t end) { // [begin, end)
    std::unordered_map<JCOEF, int> frequencyMap;

    // Count the frequency of each element in the array
    for (int i = begin; i < end; ++i) {
        frequencyMap[arr[i]]++;
    }

    // Find the minimum frequency value
    int minFrequency = std::min_element(frequencyMap.begin(), frequencyMap.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        })->second;

    // Find all elements with the minimum frequency value
    std::vector<JCOEF> leastFrequentElements;
    for (const auto& pair : frequencyMap) {
        if (pair.second == minFrequency) {
            leastFrequentElements.push_back(pair.first);
        }
    }
    std::transform(leastFrequentElements.begin(), leastFrequentElements.end(), leastFrequentElements.begin(), [](JCOEF n) { return std::abs(n); });
    JCOEF ans = *std::min_element(leastFrequentElements.begin(), leastFrequentElements.end());
    if (ans < 3)
        ans = 3;
    if (ans > 20)
        ans = 20;
    return ans;
}