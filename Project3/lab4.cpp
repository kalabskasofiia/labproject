#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <random>
#include <iomanip>

// Варіант №1: m=2 поля
// Поле 0: read 10%, write 5%
// Поле 1: read 50%, write 5%
// string: 30%

class OptimizedDataStructure {
private:
    int field0;
    int field1;
    mutable std::shared_mutex mutex0;  
    mutable std::shared_mutex mutex1;  

public:
    OptimizedDataStructure() : field0(0), field1(0) {}

    int getField0() const {
        std::shared_lock<std::shared_mutex> lock(mutex0);
        return field0;
    }

    void setField0(int value) {
        std::unique_lock<std::shared_mutex> lock(mutex0);
        field0 = value;
    }

    int getField1() const {
        std::shared_lock<std::shared_mutex> lock(mutex1);
        return field1;
    }

    void setField1(int value) {
        std::unique_lock<std::shared_mutex> lock(mutex1);
        field1 = value;
    }

    operator std::string() const {
        
        std::shared_lock<std::shared_mutex> lock0(mutex0);
        std::shared_lock<std::shared_mutex> lock1(mutex1);

        std::ostringstream oss;
        oss << "Field0: " << field0 << ", Field1: " << field1;
        return oss.str();
    }
};

class FileGenerator {
private:
    std::mt19937 gen;

public:
    FileGenerator() : gen(std::random_device{}()) {}

    void generateFile(const std::string& filename, int numOperations,
        double readField0, double writeField0,
        double readField1, double writeField1,
        double stringOp) {
        std::ofstream file(filename);
        std::uniform_real_distribution<> dis(0.0, 1.0);

        for (int i = 0; i < numOperations; ++i) {
            double rand = dis(gen);
            double cumulative = 0.0;

            cumulative += readField0;
            if (rand < cumulative) {
                file << "read 0\n";
                continue;
            }

            cumulative += writeField0;
            if (rand < cumulative) {
                file << "write 0 1\n";
                continue;
            }

            cumulative += readField1;
            if (rand < cumulative) {
                file << "read 1\n";
                continue;
            }

            cumulative += writeField1;
            if (rand < cumulative) {
                file << "write 1 1\n";
                continue;
            }

            file << "string\n";
        }
        file.close();
    }

    void generateOptimalFiles(int numOps) {
        generateFile("optimal_thread1.txt", numOps, 0.10, 0.05, 0.50, 0.05, 0.30);
        generateFile("optimal_thread2.txt", numOps, 0.10, 0.05, 0.50, 0.05, 0.30);
        generateFile("optimal_thread3.txt", numOps, 0.10, 0.05, 0.50, 0.05, 0.30);
    }

    void generateEqualFiles(int numOps) {
        double equal = 0.20;
        generateFile("equal_thread1.txt", numOps, equal, equal, equal, equal, equal);
        generateFile("equal_thread2.txt", numOps, equal, equal, equal, equal, equal);
        generateFile("equal_thread3.txt", numOps, equal, equal, equal, equal, equal);
    }

   void generateWorstFiles(int numOps) {
        generateFile("worst_thread1.txt", numOps, 0.05, 0.40, 0.05, 0.40, 0.10);
        generateFile("worst_thread2.txt", numOps, 0.05, 0.40, 0.05, 0.40, 0.10);
        generateFile("worst_thread3.txt", numOps, 0.05, 0.40, 0.05, 0.40, 0.10);
    }
};

class OperationExecutor {
private:
    OptimizedDataStructure& dataStruct;
    std::string filename;
    long long duration;

public:
    OperationExecutor(OptimizedDataStructure& ds, const std::string& file)
        : dataStruct(ds), filename(file), duration(0) {
    }

    void execute() {
        std::ifstream file(filename);
        std::vector<std::tuple<std::string, int, int>> operations;

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string op;
            iss >> op;

            if (op == "read") {
                int fieldNum;
                iss >> fieldNum;
                operations.push_back({ op, fieldNum, 0 });
            }
            else if (op == "write") {
                int fieldNum, value;
                iss >> fieldNum >> value;
                operations.push_back({ op, fieldNum, value });
            }
            else if (op == "string") {
                operations.push_back({ op, 0, 0 });
            }
        }
        file.close();

        auto start = std::chrono::high_resolution_clock::now();

        for (const auto& [op, fieldNum, value] : operations) {
            if (op == "read") {
                if (fieldNum == 0) {
                    volatile int val = dataStruct.getField0();
                    (void)val;
                }
                else {
                    volatile int val = dataStruct.getField1();
                    (void)val;
                }
            }
            else if (op == "write") {
                if (fieldNum == 0) {
                    dataStruct.setField0(value);
                }
                else {
                    dataStruct.setField1(value);
                }
            }
            else if (op == "string") {
                volatile std::string str = std::string(dataStruct);
                (void)str;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    long long getDuration() const { return duration; }
};

void runTest(const std::string& testName, const std::vector<std::string>& files) {
    std::cout << "\n    " << testName << std::endl;

    OptimizedDataStructure ds;
    std::vector<std::thread> threads;
    std::vector<OperationExecutor*> executors;

    for (const auto& file : files) {
        executors.push_back(new OperationExecutor(ds, file));
    }

    auto totalStart = std::chrono::high_resolution_clock::now();

    for (auto* exec : executors) {
        threads.emplace_back(&OperationExecutor::execute, exec);
    }

    for (auto& t : threads) {
        t.join();
    }

    auto totalEnd = std::chrono::high_resolution_clock::now();
    long long totalTime = std::chrono::duration_cast<std::chrono::microseconds>(totalEnd - totalStart).count();

    std::cout << "Кількість потоків: " << files.size() << std::endl;
    for (size_t i = 0; i < executors.size(); ++i) {
        std::cout << "  Потік " << (i + 1) << " (" << files[i] << "): "
            << std::fixed << std::setprecision(2)
            << executors[i]->getDuration() / 1000.0 << " мс" << std::endl;
    }
    std::cout << "Загальний час: " << std::fixed << std::setprecision(2)
        << totalTime / 1000.0 << " мс" << std::endl;

    for (auto* exec : executors) {
        delete exec;
    }
}

int main() {
    system("chcp 65001");

    const int NUM_OPERATIONS = 100000;

    std::cout << "Генерація файлів..." << std::endl;
    FileGenerator generator;

    generator.generateOptimalFiles(NUM_OPERATIONS);
  
    generator.generateEqualFiles(NUM_OPERATIONS);

    generator.generateWorstFiles(NUM_OPERATIONS);

    std::cout << "Файли згенеровано успішно!\n" << std::endl;

    // Тести для оптимальних частот
    std::cout << "\nОПТИМАЛЬНІ ЧАСТОТИ (варіант №1)" << std::endl;
    runTest("1 потік", { "optimal_thread1.txt" });
    runTest("2 потоки", { "optimal_thread1.txt", "optimal_thread2.txt" });
    runTest("3 потоки", { "optimal_thread1.txt", "optimal_thread2.txt", "optimal_thread3.txt" });

    // Тести для рівних частот
    std::cout << "\nРІВНІ ЧАСТОТИ" << std::endl;
    runTest("1 потік", { "equal_thread1.txt" });
    runTest("2 потоки", { "equal_thread1.txt", "equal_thread2.txt" });
    runTest("3 потоки", { "equal_thread1.txt", "equal_thread2.txt", "equal_thread3.txt" });

    // Тести для найгірших частот
    std::cout << "\nНАЙГІРШІ ЧАСТОТИ (багато write)" << std::endl;
    runTest("1 потік", { "worst_thread1.txt" });
    runTest("2 потоки", { "worst_thread1.txt", "worst_thread2.txt" });
    runTest("3 потоки", { "worst_thread1.txt", "worst_thread2.txt", "worst_thread3.txt" });

    return 0;
}