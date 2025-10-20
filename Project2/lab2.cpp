// Компілятор: MSVC 19.3x

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <execution>
#include <thread>
#include <cmath>
#include <functional>
#include <iomanip>
#include <numeric>

using namespace std;

template <typename Function>
double MeasureExecutionTime(Function&& f) {
    auto start = chrono::high_resolution_clock::now();
    f();
    auto end = chrono::high_resolution_clock::now();
    return chrono::duration<double, milli>(end - start).count();
}

// Послідовний підрахунок середнього абсолютних різниць
double CalculateMeanDiff(const vector<double>& data) {
    if (data.size() < 2) return 0.0;

    double sum = 0.0;
    for (size_t i = 1; i < data.size(); i++) {
        sum += abs(data[i] - data[i - 1]);
    }
    return sum / (data.size() - 1);
}

// Паралельна версія з K потоками
double ParallelMeanDiff(const vector<double>& data, int K) {
    if (data.size() < 2) return 0.0;

    size_t n = data.size();
    size_t edges = n - 1;
    size_t chunkSize = (edges + K - 1) / K;

    vector<thread> threads;
    vector<double> partialSums(K, 0.0);

    for (int k = 0; k < K; k++) {
        threads.emplace_back([&, k]() {
            size_t start = k * chunkSize + 1;
            size_t end = min(start + chunkSize, n);

            double localSum = 0.0;
            for (size_t i = start; i < end; i++) {
                localSum += abs(data[i] - data[i - 1]);
            }
            partialSums[k] = localSum;
            });
    }

    for (auto& t : threads) {
        t.join();
    }

    double total = 0.0;
    for (double s : partialSums) {
        total += s;
    }

    return total / edges;
}

void AnalyzeMeanDiffPerformance(const vector<double>& data) {
    int maxThreads = thread::hardware_concurrency();
    double bestTime = numeric_limits<double>::max();
    int bestK = 0;

    cout << "\n\nЧастина 2: K-way паралелізація\n";
    cout << "================================\n";
    cout << "Розмір даних: " << data.size() << "\n\n";
    cout << setw(5) << "K" << setw(15) << "Час (мс)" << setw(20) << "Результат\n";
    cout << string(40, '-') << "\n";

    for (int K = 1; K <= maxThreads * 2; K++) {
        double result = 0.0;
        double time = MeasureExecutionTime([&]() {
            result = ParallelMeanDiff(data, K);
            });

        cout << setw(5) << K << setw(15) << fixed << setprecision(6) << time
            << setw(20) << fixed << setprecision(4) << result << "\n";

        if (time < bestTime) {
            bestTime = time;
            bestK = K;
        }
    }

    cout << "\n\nВисновки:\n";
    cout << "---------\n";
    cout << "Найкраще K: " << bestK << "\n";
    cout << "Найкращий час: " << fixed << setprecision(6) << bestTime << " мілісекунд\n";
    cout << "Кількість потоків процесора: " << maxThreads << "\n";
    cout << "Співвідношення K/потоки: " << fixed << setprecision(2)
        << (double)bestK / maxThreads << "\n";
}

int main() {
    system("chcp 65001 ");

    cout << "Дослідження adjacent_difference\n";
    cout << "================================\n\n";

    mt19937_64 rng(42);
    uniform_real_distribution<double> dist(-100000.0, 100000.0);

    vector<size_t> sizes = { 10000, 100000, 1000000, 5000000 };

    cout << "Частина 1: Порівняння різних підходів\n";
    cout << "--------------------------------------\n";

    for (size_t n : sizes) {
        vector<double> data(n);
        for (auto& x : data) {
            x = dist(rng);
        }

        cout << "\nРозмір даних: " << n << "\n";

        // Без політики
        vector<double> diffs(n);
        double result1 = 0.0;
        double time1 = MeasureExecutionTime([&]() {
            adjacent_difference(data.begin(), data.end(), diffs.begin());
            double sum = 0.0;
            for (size_t i = 1; i < diffs.size(); i++) {
                sum += abs(diffs[i]);
            }
            result1 = sum / (n - 1);
            });
        cout << "Без політики: " << time1 << " мс\n";

        // execution::seq
        double result2 = 0.0;
        double time2 = MeasureExecutionTime([&]() {
            adjacent_difference(execution::seq, data.begin(), data.end(), diffs.begin());
            double sum = 0.0;
            for (size_t i = 1; i < diffs.size(); i++) {
                sum += abs(diffs[i]);
            }
            result2 = sum / (n - 1);
            });
        cout << "execution::seq: " << time2 << " мс\n";

        // execution::par
        double result3 = 0.0;
        double time3 = MeasureExecutionTime([&]() {
            adjacent_difference(execution::par, data.begin(), data.end(), diffs.begin());
            double sum = 0.0;
            for (size_t i = 1; i < diffs.size(); i++) {
                sum += abs(diffs[i]);
            }
            result3 = sum / (n - 1);
            });
        cout << "execution::par: " << time3 << " мс\n";

        // execution::par_unseq
        double result4 = 0.0;
        double time4 = MeasureExecutionTime([&]() {
            adjacent_difference(execution::par_unseq, data.begin(), data.end(), diffs.begin());
            double sum = 0.0;
            for (size_t i = 1; i < diffs.size(); i++) {
                sum += abs(diffs[i]);
            }
            result4 = sum / (n - 1);
            });
        cout << "execution::par_unseq: " << time4 << " мс\n";
    }

    // K-way аналіз на найбільшому розмірі
    size_t testSize = sizes.back();
    vector<double> testData(testSize);
    for (auto& x : testData) {
        x = dist(rng);
    }

    AnalyzeMeanDiffPerformance(testData);

    return 0;
}