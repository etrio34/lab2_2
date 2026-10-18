// Робота з файлами послідовностей.
// Формат файлу: перший рядок -- кількість елементів N, далі N цілих чисел
// (long long), розділених пробілами (по 20 в рядку).
#pragma once
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

inline std::string sequenceFileName(std::size_t n) {
    return "seq_" + std::to_string(n) + ".txt";
}

inline void writeSequence(const std::string& path, const std::vector<long long>& v) {
    std::ofstream out(path);
    out << v.size() << '\n';
    for (std::size_t i = 0; i < v.size(); ++i)
        out << v[i] << ((i + 1) % 20 == 0 ? '\n' : ' ');
    out << '\n';
}

inline std::vector<long long> readSequence(const std::string& path) {
    std::ifstream in(path);
    std::size_t n = 0;
    if (!(in >> n) || n == 0)
        throw std::runtime_error("Cannot read sequence file: " + path +
                                 " (run gen_data in the working directory first)");
    std::vector<long long> v(n);
    for (auto& x : v) in >> x;
    return v;
}
