// Генерація послідовностей для експериментів (бібліотека <random>).
// Значення з [1, 1000000006] -- придатні як для додавання, так і для множення за модулем 1e9+7.
#include <random>
#include <vector>
#include "data_io.h"

int main() {
    const std::vector<std::size_t> sizes = {100'000, 1'000'000, 10'000'000};
    std::mt19937_64 gen(12345);
    std::uniform_int_distribution<long long> dist(1, 1'000'000'006LL);
    for (auto n : sizes) {
        std::vector<long long> v(n);
        for (auto& x : v) x = dist(gen);
        writeSequence(sequenceFileName(n), v);
    }
}
