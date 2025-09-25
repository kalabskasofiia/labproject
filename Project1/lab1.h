#pragma once
#include <random>
#include <vector>

using namespace std;

class WeightedRandom
{
	vector<int> values;
	vector<int> weights;
	mt19937 gen;
	discrete_distribution<> dist;

public: 
	WeightedRandom(const vector <int>& v, const vector <int>& w);
	int operator()();

};

