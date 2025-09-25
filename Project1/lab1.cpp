#include "lab1.h"
#include <stdexcept>

using namespace std;

lab1::WeightedRandom(const vector<int>& v, const vector<int>& w)
	: values(v), weights(w), gen(std::random_device{}()), dist(w.begin(), w.end()) {
	if (v.size() != w.size() || v.empty()) {
		throw invalid_argument("Некоректні вхідні дані");
	}
}

int lab1::operator()() {
	ind idx = dist(gen);
	return values[idx];
}