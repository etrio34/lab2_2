// Допоміжні засоби вимірювання часу.
#pragma once
#include <algorithm>
#include <chrono>
#include <vector>

// Повертає мінімальний з `repeats` запусків, мс
template <class F>
double measureMs(F&& f, int repeats) {
    double best = 1e300;
    for (int r = 0; r < repeats; ++r) {
        auto t0 = std::chrono::steady_clock::now();
        f();
        auto t1 = std::chrono::steady_clock::now();
        best = std::min(best, std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    return best;
}
