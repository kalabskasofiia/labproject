#include "lab1.h"
#include <stdexcept>

WeightedRandom::WeightedRandom(const std::vector<int>& v, const std::vector<int>& w)
	: values(v), weights(w), gen(std::random_device{}()), dist(w.begin(), w.end()) {
	if (v.size() != w.size() || v.empty()) {
		throw std::invalid_argument("Некоректні вхідні дані");
	}
}

int WeightedRandom::operator()() {
	ind idx = dist(gen);
	return values[idx];
}