// Власний паралельний inclusive_scan на K потоках.
// 1) вибірка ділиться на K приблизно рівних частин; кожна частина
//    обробляється в окремому потоці послідовним std::inclusive_scan;
// 2) підсумки частин (останні елементи) зводяться послідовним
//    std::inclusive_scan -> префікси підсумків;
// 3) до кожної частини, крім першої, застосовується префікс попередніх
//    частин (std::transform у окремих потоках). Для scan цей крок
//    неминучий: без нього результат невірний.
// Операція op має бути асоціативною.
#pragma once
#include <algorithm>
#include <numeric>
#include <thread>
#include <vector>

template <class T, class Op>
void parallelInclusiveScan(const std::vector<T>& in, std::vector<T>& out,
                           std::size_t K, Op op) {
    const std::size_t n = in.size();
    if (n == 0) return;
    K = std::max<std::size_t>(1, std::min(K, n));
    const std::size_t base = n / K, extra = n % K;

    std::vector<std::size_t> begin(K + 1, 0);           // межі частин
    for (std::size_t i = 0; i < K; ++i)
        begin[i + 1] = begin[i] + base + (i < extra ? 1 : 0);

    auto runInThreads = [K](auto&& task) {
        std::vector<std::thread> threads;
        threads.reserve(K);
        for (std::size_t i = 0; i < K; ++i) threads.emplace_back(task, i);
        for (auto& t : threads) t.join();
    };

    runInThreads([&](std::size_t i) {                   // крок 1
        std::inclusive_scan(in.begin() + begin[i], in.begin() + begin[i + 1],
                            out.begin() + begin[i], op);
    });

    std::vector<T> tails(K);                            // крок 2
    for (std::size_t i = 0; i < K; ++i) tails[i] = out[begin[i + 1] - 1];
    std::inclusive_scan(tails.begin(), tails.end(), tails.begin(), op);

    runInThreads([&](std::size_t i) {                   // крок 3
        if (i == 0) return;
        const T offset = tails[i - 1];
        std::transform(out.begin() + begin[i], out.begin() + begin[i + 1],
                       out.begin() + begin[i],
                       [&](const T& x) { return op(offset, x); });
    });
}
