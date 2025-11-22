#include <iostream>
#include <thread>
#include <vector>
#include <latch>
#include <syncstream>
#include <chrono>

using namespace std;

latch a_done(1);
latch b_done(1);
latch c_done(1);
latch d_done(1);
latch e_done(1);
latch f_done(1);
latch g_done(1);
latch h_done(1);
latch i_done(1);

void run_action(char name, int count) {
    for (int step = 1; step <= count; ++step) {
        this_thread::sleep_for(chrono::milliseconds(10));
        osyncstream(cout) << "From set " << name << " done action " << step << "." << endl;
    }
}

void thread1_work() {
    run_action('a', 6);
    a_done.count_down();

    run_action('c', 8);
    c_done.count_down();
}

void thread2_work() {
    run_action('b', 5);
    b_done.count_down();

    run_action('e', 8);
    e_done.count_down();
}

void thread3_work() {
    a_done.wait();
    run_action('d', 5);
    d_done.count_down();

    e_done.wait();
    run_action('h', 5);
    h_done.count_down();
}

void thread4_work() {
    b_done.wait();
    run_action('f', 9);
    f_done.count_down();

    run_action('i', 6);
    i_done.count_down();
}

void thread5_work() {
    c_done.wait();
    run_action('g', 4);
    g_done.count_down();

    h_done.wait();
    i_done.wait();
    run_action('j', 8);
}

int main() {
    osyncstream(cout) << "counting start" << endl;

    auto start_time = chrono::high_resolution_clock::now();

    thread t1(thread1_work);
    thread t2(thread2_work);
    thread t3(thread3_work);
    thread t4(thread4_work);
    thread t5(thread5_work);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();

    osyncstream(cout) << "counting end" << endl;
    osyncstream(cout) << "Execution time: " << duration << " ms" << endl;

    return 0;
}