#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <iomanip>
#include <stdexcept>
#include "lab1.h"
using namespace std;

int main() {
    system("chcp 65001 ");

    try {
        string filename;
        cout << "Введіть назву файлу: ";
        cin >> filename;

        ifstream fin(filename);
        if (!fin) {
            throw runtime_error("Не вдалося відкрити файл!");
        }

        int N;
        fin >> N; // кількість генерацій
        if (!fin || N <= 0) {
            throw runtime_error("Некоректне N (має бути натуральним числом).");
        }

        vector<int> values, weights;
        int x;

        // читаємо числа
        while (fin >> x) {
            values.push_back(x);
            if (fin.peek() == '\n') break;
        }

        // читаємо ваги
        while (fin >> x) {
            weights.push_back(x);
        }

        if (values.size() != weights.size() || values.empty()) {
            throw runtime_error("Кількість чисел і частот не співпадає або вони порожні.");
        }

        WeightedRandom generator(values, weights);

        map<int, int> freq;
        for (int i = 0; i < N; i++) {
            int val = generator();
            freq[val]++;
        }

        cout << "\nРезультати генерації:\n";
        cout << setw(10) << "Число"
            << setw(15) << "Задана частота"
            << setw(20) << "Отримана частота\n";

        double maxDiff = 0.0;
        int sumWeights = 0;
        for (int w : weights) sumWeights += w;

        for (size_t i = 0; i < values.size(); i++) {
            double expected = (double)weights[i] / sumWeights;
            double real = (double)freq[values[i]] / N;
            maxDiff = max(maxDiff, abs(expected - real));

            cout << setw(10) << values[i]
                << setw(15) << fixed << setprecision(3) << expected
                << setw(20) << fixed << setprecision(3) << real << "\n";
        }

        cout << "\nНайбільша розбіжність: " << fixed << setprecision(3) << maxDiff << "\n";
    }
    catch (const exception& e) {
        cout << "Помилка: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        cout << "Невідома помилка!\n";
        return 1;
    }

    return 0;
}