#include <iostream>
#include <chrono>

using namespace std::chrono;

int main() {
    auto a = 90s;
    ++a;
    a *= 2;

    auto b = 3min;
    auto c = a % b;

    auto d = std::chrono::abs(-10s);

    auto cmp = (c + d <=> 15s);

    std::cout << (cmp < 0 ? "A" : "B") << "\n";
    std::cout << c.count() << " " << d.count() << "\n";

    return 0;
}