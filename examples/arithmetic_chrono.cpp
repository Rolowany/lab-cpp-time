#include <iostream>
#include <chrono>
#include <thread>

// by oszczędzić męki piszemy
using namespace std::chrono;

int main() {
    // timepoint nr 1
    auto tp1 = steady_clock::now();

    std::this_thread::sleep_for(milliseconds(100));

    // timepoint nr 2
    auto tp2 = steady_clock::now();

    // duration nr 1 i 2
    seconds d1(5);
    hours d2(2);

    auto elapsed = tp2 - tp1;
    std::cout << "1. time_point - time_point -> duration: "
              << duration_cast<milliseconds>(elapsed).count() << " ms\n";

    time_point<steady_clock> tp_future = tp1 + d1;
    std::cout << "2. time_point + duration -> time_point\n";

    time_point<steady_clock> tp_past = tp1 - d2;
    std::cout << "3. time_point - duration -> time_point\n";

    auto d_sum = d1 + d2;
    auto d_diff = d1 - d2;
    std::cout << "4. duration + duration -> duration: " << d_sum.count() << " s\n";
    std::cout << "4. duration - duration -> duration: " << d_diff.count() << " s\n";

    auto d_multiplied = d1 * 3;
    auto d_divided = d1 / 2;
    std::cout << "5. duration * skalar -> duration: " << d_multiplied.count() << " s\n";
    std::cout << "5. duration / skalar -> duration: " << d_divided.count() << " s\n";

    auto ratio = d1 / d2;
    std::cout << "6. duration / duration -> liczba: " << ratio << "\n";
}