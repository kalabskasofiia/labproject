#include "lab1.h"
#include <stdexcept>

using namespace std;

WeightedRandom::WeightedRandom(const vector<int>& v, const vector<int>& w)
    : values(v), weights(w), gen(std::random_device{}()), dist(w.begin(), w.end()) {
    if (values.empty() || weights.empty()) {
        throw std::invalid_argument("Некоректні вхідні дані");
    }
}

int WeightedRandom::operator()() {
	int idx = dist(gen);
	return values[idx];
}