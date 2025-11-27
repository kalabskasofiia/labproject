#include <iostream>
#include <coroutine>
#include <queue>
#include <random>
#include <thread>
#include <chrono>

using namespace std;

class QueueAwaiter {
private:
    queue<int>& q;
    const size_t maxSize = 32;

public:
    QueueAwaiter(queue<int>& queue) : q(queue) {}

    bool await_ready() const noexcept {
        return q.size() < maxSize;
    }

    void await_suspend(coroutine_handle<> handle) noexcept {
        cout << "  [ПРИЗУПИНЕНО] Черга переповнена (" << q.size()
            << " елементів). Чекаємо звільнення місця..." << endl;
    }

    void await_resume() noexcept {
        cout << "  [ВІДНОВЛЕНО] Черга звільнилась (" << q.size()
            << " елементів). Продовжуємо генерацію." << endl;
    }
};

struct RandomGenerator {
    struct promise_type {
        RandomGenerator get_return_object() {
            return RandomGenerator{ coroutine_handle<promise_type>::from_promise(*this) };
        }

        suspend_never initial_suspend() { return {}; }
        suspend_always final_suspend() noexcept { return {}; }

        void return_void() {}
        void unhandled_exception() { terminate(); }
    };

    coroutine_handle<promise_type> handle;

    RandomGenerator(coroutine_handle<promise_type> h) : handle(h) {}

    ~RandomGenerator() {
        if (handle) handle.destroy();
    }

    RandomGenerator(const RandomGenerator&) = delete;
    RandomGenerator& operator=(const RandomGenerator&) = delete;

    RandomGenerator(RandomGenerator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    bool resume() {
        if (!handle.done()) {
            handle.resume();
            return !handle.done();
        }
        return false;
    }

    bool done() const {
        return handle.done();
    }
};

RandomGenerator generateRandomNumbers(queue<int>& q, int count) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 256);

    cout << "\n[СОПРОГРАМА ЗАПУЩЕНА] Генерація " << count << " випадкових чисел [1, 256]" << endl;
    cout << "Правило: призупинення коли черга >= 32 елементів\n" << endl;

    for (int i = 0; i < count; ++i) {
        co_await QueueAwaiter(q);

        int randomNum = dist(gen);
        q.push(randomNum);

        cout << "  [" << (i + 1) << "/" << count << "] Згенеровано: "
            << randomNum << " | Розмір черги: " << q.size() << endl;

        this_thread::sleep_for(chrono::milliseconds(30));
    }

    cout << "\n[СОПРОГРАМА ЗАВЕРШЕНА] Всі числа згенеровані." << endl;
}

int main() {
    system("chcp 65001 > nul");

    cout << "ДЕМО: Сопрограма з контролем черги\n" << endl;

    queue<int> numberQueue;
    const int TOTAL_NUMBERS = 100;
    const int CONSUME_BATCH = 25;

    auto generator = generateRandomNumbers(numberQueue, TOTAL_NUMBERS);

    int consumed = 0;
    int iteration = 1;

    while (!generator.done() || !numberQueue.empty()) {

        if (!generator.done()) {
            generator.resume();
        }

        if (!numberQueue.empty()) {
            cout << "\n--- [СПОЖИВАЧ] Ітерація " << iteration << " ---" << endl;

            int toConsume = min(CONSUME_BATCH, (int)numberQueue.size());

            cout << "Елементів у черзі: " << numberQueue.size() << endl;
            cout << "Споживаємо: " << toConsume << " елементів\n" << endl;

            for (int i = 0; i < toConsume; ++i) {
                int num = numberQueue.front();
                numberQueue.pop();
                consumed++;

                if (i < 5 || i >= toConsume - 2) {
                    cout << "  Спожито: " << num
                        << " (залишилось: " << numberQueue.size() << ")" << endl;
                }
                else if (i == 5) {
                    cout << "  ... (спожито ще " << (toConsume - 7) << " елементів) ..." << endl;
                }
            }

            cout << "\nСпожито в цій ітерації: " << toConsume << endl;
            cout << "Всього спожито: " << consumed << "/" << TOTAL_NUMBERS << endl;

            iteration++;

            this_thread::sleep_for(chrono::milliseconds(150));
        }
    }

    cout << "ПІДСУМОК" << endl;
    cout << "Згенеровано: " << TOTAL_NUMBERS << " чисел" << endl;
    cout << "Спожито: " << consumed << " чисел" << endl;
    cout << "Залишилось у черзі: " << numberQueue.size() << endl;
    cout << "Ітерацій споживання: " << (iteration - 1) << endl;

    return 0;
}