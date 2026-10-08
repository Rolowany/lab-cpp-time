#include <iostream>
#include <ctime>
#include <thread>
#include <vector>

int main() {
    // Jak wyświetlić aktualną datę?
    std::time_t now = std::time(nullptr);
    std::tm* lt = std::localtime(&now);
    char buf[64];
    std::strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", lt);

    std::cout << "Aktualny czas: " << buf << std::endl;

    // Jak zmierzyć czas jakiegoś procesu?
    std::clock_t start_clock = std::clock();

    volatile long long sum = 0;
    for (long long i = 0; i < 100000000; ++i) {
        sum += i;
    }

    std::clock_t end_clock = std::clock();
    double cpu_time = double(end_clock - start_clock) / CLOCKS_PER_SEC;
    std::cout << "Czas procesora: " << cpu_time << " s" << std::endl;

    // ctime mierzy czas procesora a nie czas rzeczywisty !!!
    start_clock = std::clock();

    sum = 0;
    for (long long i = 0; i < 100000000; ++i) {
        sum += i;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    end_clock = std::clock();
    cpu_time = double(end_clock - start_clock) / CLOCKS_PER_SEC;
    std::cout << "Czas procesora, ale bynajmniej nie czas rzeczywisty!!!: " << cpu_time << " s" << std::endl;

    // Pokazanie, że ctime nie nadaje się do programowania równoległego
    // Funkcje z <ctime> zwracają
    // wskaźniki do statycznych, współdzielonych struktur/buforów. Wywołanie ich
    // jednocześnie z wielu wątków powoduje wyścig danych (data race) i nadpisywanie wyników.
    auto unsafe_func = []() {
        std::time_t t = std::time(nullptr);
        char* str_time = std::ctime(&t);
    };

    std::vector<std::thread> pool;
    for (int i = 0; i < 4; ++i) {
        pool.emplace_back(unsafe_func);
    }
    for (auto& t : pool) {
        t.join();
    }
    std::cout << "Wątki zakończyły działanie (potencjalny data race w funkcjach ctime)." << std::endl;
}