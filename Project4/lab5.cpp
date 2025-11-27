#include <iostream>
#include <string>
#include <syncstream>
#include <future>
#include <chrono>

using namespace std;

void quick(const string& name) {
    this_thread::sleep_for(chrono::seconds(1));
    osyncstream(cout) << name << " (quick, 1s)" << endl;
}

void slow(const string& name) {
    this_thread::sleep_for(chrono::seconds(7));
    osyncstream(cout) << name << " (slow, 7s)" << endl;
}

void work() {
    auto start = chrono::high_resolution_clock::now();

    osyncstream(cout) << "Starting computations" << endl;

    future<void> fA1 = async(launch::async, quick, "A1");
    future<void> fA2 = async(launch::async, slow, "A2");
    future<void> fC1 = async(launch::async, quick, "C1");
    future<void> fC2 = async(launch::async, quick, "C2");

    fA1.wait();
    fA2.wait();

    future<void> fB = async(launch::async, slow, "B");

    fB.wait();
    fC1.wait();

    fC2.wait();

    future<void> fD1 = async(launch::async, quick, "D1");
    future<void> fD2 = async(launch::async, quick, "D2");

    fD1.wait();
    fD2.wait();

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    osyncstream(cout) << "\nAll computations completed" << endl;
    osyncstream(cout) << "Time: " << elapsed.count() << " seconds" << endl;
    osyncstream(cout) << "Work is done!" << endl;
}

int main() {
    work();
    return 0;
}