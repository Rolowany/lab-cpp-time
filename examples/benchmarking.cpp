#include <chrono>
#include <iostream>
#include <bits/this_thread_sleep.h>

void do_work() {
    volatile long long sum = 0;
    for (long long i = 0; i < 100000000; ++i) {
        sum += i;
    }
}

class ScopedTimer {
public:
    explicit ScopedTimer(std::string_view name)
        : name_{name}, start_{std::chrono::steady_clock::now()} {}

    ~ScopedTimer() {
        using namespace std::chrono;
        const duration<double, std::milli> dt = steady_clock::now() - start_;
        std::cout << name_ << ": " << dt.count() << " ms\n";
    }

private:
    std::string_view name_;
    std::chrono::steady_clock::time_point start_;
};

int main() {
    using clock = std::chrono::steady_clock;

    const auto start = clock::now();
    do_work();
    const auto elapsed = clock::now() - start;

    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count()
              << " ms\n";

    std::chrono::duration<double, std::milli> ms = elapsed;
    std::cout << ms.count() << " ms\n";

    std::cout << elapsed << '\n';

    std::cout << "Zmierzone przy pomocy steady_clock\n";

    ScopedTimer timer("do_work");

    // Można odkomentować i sprawdzić czy doda dodatkową sekundę
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    do_work();
}