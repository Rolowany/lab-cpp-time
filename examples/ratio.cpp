#include <iostream>
#include <ratio>

int main() {
    using r1 = std::ratio<1, 3>;
    using r2 = std::ratio<2, 6>;

    std::cout << "r1: " << r1::num << "/" << r1::den << "\n";
    std::cout << "r2: " << r2::num << "/" << r2::den << "\n";

    std::cout << "std::kilo::num / den: " << std::kilo::num << "/" << std::kilo::den << " (1000)\n";
    std::cout << "std::milli::num / den: " << std::milli::num << "/" << std::milli::den << " (0.001)\n\n";

    using sum = std::ratio_add<std::ratio<1, 4>, std::ratio<1, 6>>;
    std::cout << "1/4 + 1/6 = " << sum::num << "/" << sum::den << "\n";

    using product = std::ratio_multiply<std::ratio<2, 3>, std::ratio<3, 4>>;
    std::cout << "2/3 * 3/4 = " << product::num << "/" << product::den << "\n\n";

    constexpr bool equal = std::ratio_equal<r1, r2>::value;
    constexpr bool smaller = std::ratio_less<std::ratio<1, 5>, std::ratio<1, 3>>::value;

    std::cout << std::boolalpha;
    std::cout << "Czy r1 == r2? " << equal << "\n";
    std::cout << "Czy 1/5 < 1/3? " << smaller << "\n";

    return 0;
}