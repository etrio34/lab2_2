// Variant 12: std::inclusive_scan.
// Compiler: g++
// Input: files seq_<N>.txt (format described in data_io.hpp), created by gen_data.
#include <execution>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>
#include "benchmark.h"
#include "data_io.h"
#include "parallel_scan.h"

namespace {

constexpr long long MOD = 1'000'000'007LL;
constexpr int REPEATS = 5;

// Two binary operations (both associative):
struct Add {                       // fast
    long long operator()(long long a, long long b) const { return a + b; }
};
struct MulMod {                    // "heavy": modulo division
    long long operator()(long long a, long long b) const { return (a * b) % MOD; }
};

template <class Op>
void libraryExperiments(const std::vector<long long>& in, Op op, const char* opName) {
    std::vector<long long> out(in.size());
    auto line = [&](const char* name, double ms) {
        std::cout << "  " << std::left << std::setw(26) << name
                  << std::right << std::setw(10) << ms << " ms\n";
    };
    std::cout << " [operation " << opName << ", N=" << in.size() << "]\n";
    line("no execution policy", measureMs([&] {
        std::inclusive_scan(in.begin(), in.end(), out.begin(), op); }, REPEATS));
    line("execution::seq", measureMs([&] {
        std::inclusive_scan(std::execution::seq, in.begin(), in.end(), out.begin(), op); }, REPEATS));
    line("execution::par", measureMs([&] {
        std::inclusive_scan(std::execution::par, in.begin(), in.end(), out.begin(), op); }, REPEATS));
    line("execution::par_unseq", measureMs([&] {
        std::inclusive_scan(std::execution::par_unseq, in.begin(), in.end(), out.begin(), op); }, REPEATS));
}

const std::vector<std::size_t> kValues = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 64, 128, 256, 512, 1024};

// Prints the "K -> time" table for the given operation and the best K.
template <class Op>
void ownExperiments(const std::vector<std::vector<long long>>& data, Op op, const char* opName) {
    std::cout << "\n=== Custom algorithm, operation " << opName << ": time (ms) depending on K ===\n";
    std::cout << std::setw(6) << "K";
    for (auto& v : data) std::cout << std::setw(12) << ("N=" + std::to_string(v.size()));
    std::cout << '\n';

    std::vector<double> bestTime(data.size(), 1e300);
    std::vector<std::size_t> bestK(data.size(), 0);
    for (auto K : kValues) {
        std::cout << std::setw(6) << K;
        for (std::size_t d = 0; d < data.size(); ++d) {
            const auto& in = data[d];
            std::vector<long long> out(in.size());
            double ms = measureMs([&] { parallelInclusiveScan(in, out, K, op); }, REPEATS);
            std::cout << std::setw(12) << ms;
            if (ms < bestTime[d]) { bestTime[d] = ms; bestK[d] = K; }
        }
        std::cout << '\n';
    }
    const unsigned hw = std::thread::hardware_concurrency();
    for (std::size_t d = 0; d < data.size(); ++d)
        std::cout << "Best K for N=" << data[d].size() << ": " << bestK[d]
                  << " (" << bestTime[d] << " ms); CPU threads: " << hw
                  << "; K/threads = " << double(bestK[d]) / hw << '\n';
}

template <class Op>
bool verifyOwnAlgorithm(const std::vector<long long>& in, Op op) {
    std::vector<long long> ref(in.size()), got(in.size());
    std::inclusive_scan(in.begin(), in.end(), ref.begin(), op);
    for (std::size_t K : {1, 3, 7, 16}) {
        parallelInclusiveScan(in, got, K, op);
        if (ref != got) return false;
    }
    return true;
}

} // namespace

int main() try {
    std::cout << std::unitbuf;
    const std::vector<std::size_t> sizes = {100'000, 1'000'000, 10'000'000};
    std::vector<std::vector<long long>> data;
    for (auto n : sizes) data.push_back(readSequence(sequenceFileName(n)));

    std::cout << "CPU threads (hardware_concurrency): "
              << std::thread::hardware_concurrency() << "\n";
    std::cout << "Verifying custom algorithm (Add, MulMod): "
              << (verifyOwnAlgorithm(data[0], Add{}) && verifyOwnAlgorithm(data[0], MulMod{})
                      ? "OK" : "FAILED") << "\n\n";

    std::cout << "=== Library inclusive_scan: execution policies ===\n";
    for (auto& v : data) {
        libraryExperiments(v, Add{}, "Add (fast)");
        libraryExperiments(v, MulMod{}, "MulMod (heavy)");
    }
    ownExperiments(data, Add{}, "Add (fast)");
    ownExperiments(data, MulMod{}, "MulMod (heavy)");
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
}
