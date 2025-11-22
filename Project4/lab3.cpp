#include <iostream>
#include <thread>
#include <vector>
#include <latch>
#include <syncstream>
#include <chrono>

using namespace std;

// Parameters: nt=5, a=6, b=5, c=8, d=5, e=8, f=9, g=4, h=5, i=6, j=8

// Latches for synchronization
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
        osyncstream(cout) << "З набору " << name << " виконано дію " << step << "." << endl;
    }
}

// Thread 1: a(6) -> c(8), d(5)
void thread1_work() {
    run_action('a', 6);
    a_done.count_down();

    run_action('c', 8);
    c_done.count_down();
}

// Thread 2: b(5) -> e(8), f(9)
void thread2_work() {
    run_action('b', 5);
    b_done.count_down();

    run_action('e', 8);
    e_done.count_down();
}

// Thread 3: a_done -> d(5) -> h(5)
void thread3_work() {
    a_done.wait();
    run_action('d', 5);
    d_done.count_down();

    e_done.wait();
    run_action('h', 5);
    h_done.count_down();
}

// Thread 4: b_done -> f(9) -> i(6)
void thread4_work() {
    b_done.wait();
    run_action('f', 9);
    f_done.count_down();

    run_action('i', 6);
    i_done.count_down();
}

// Thread 5: c_done -> g(4), then wait for g,h,i -> j(8)
void thread5_work() {
    c_done.wait();
    run_action('g', 4);
    g_done.count_down();

    h_done.wait();
    i_done.wait();
    run_action('j', 8);
}

int main() {
    system("chcp 65001");
    osyncstream(cout) << "Обчислення розпочато." << endl;

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

    osyncstream(cout) << "Обчислення завершено." << endl;
    osyncstream(cout) << "Час виконання: " << duration << " мс" << endl;

    return 0;
}