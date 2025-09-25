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
        cout << "������ ����� �����: ";
        cin >> filename;

        ifstream fin(filename);
        if (!fin) {
            throw runtime_error("�� ������� ������� ����!");
        }

        int N;
        fin >> N; // ������� ���������
        if (!fin || N <= 0) {
            throw runtime_error("���������� N (�� ���� ����������� ������).");
        }

        vector<int> values, weights;
        int x;

        // ������ �����
        while (fin >> x) {
            values.push_back(x);
            if (fin.peek() == '\n') break;
        }

        // ������ ����
        while (fin >> x) {
            weights.push_back(x);
        }

        if (values.size() != weights.size() || values.empty()) {
            throw runtime_error("ʳ������ ����� � ������ �� ������� ��� ���� ������.");
        }

        WeightedRandom generator(values, weights);

        map<int, int> freq;
        for (int i = 0; i < N; i++) {
            int val = generator();
            freq[val]++;
        }

        cout << "\n���������� ���������:\n";
        cout << setw(10) << "�����"
            << setw(15) << "������ �������"
            << setw(20) << "�������� �������\n";

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

        cout << "\n�������� ���������: " << fixed << setprecision(3) << maxDiff << "\n";
    }
    catch (const exception& e) {
        cout << "�������: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        cout << "������� �������!\n";
        return 1;
    }

    return 0;
}