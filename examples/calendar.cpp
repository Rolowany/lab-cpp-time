#include <chrono>
#include <iostream>

using namespace std::chrono;

int main() {
    // składanie dat
    auto d1 = 2024y/March/15;          // rok/miesiąc/dzień (ISO)
    auto d2 = March/15/2024;           // miesiąc/dzień/rok (USA)
    auto d3 = 15d/March/2024;          // dzień/miesiąc/rok (Europa)
    auto d4 = 2024y/February/last;  // ostatni dzień lutego
    auto d5 = 2024y/May/Monday[2]; // drugi poniedziałek maja 2024
    auto d6 = 2024y/October/Friday[last]; // ostatni piątek października 2024

    // sprawdzanie poprawności dat
    year_month_day bad = 2023y/February/29;
    bool ok = bad.ok();                             // false

    auto leap = (2024y/February/29).ok();      // true
    bool is_leap = (2024y).is_leap();               // true

    year_month_day ymd = 2024y/March/15;
    sys_days sd = ymd;                           // data -> liczba dni
    year_month_day back = sd;                       // liczba dni -> data

    weekday wd{sd};                                 // dzień tygodnia
    auto tomorrow = year_month_day{sd + days{1}};   // 2024-03-16
    auto za_tydzien = year_month_day{sd + weeks{1}};

    // liczba dni między datami
    auto diff = sys_days{2024y/December/31} - sys_days{2024y/January/1};

    hh_mm_ss t{14h + 30min + 5s + 250ms};
    t.hours();          // 14h
    t.minutes();        // 30min
    t.seconds();        // 5s
    t.subseconds();     // 250ms
    t.to_duration();    // z powrotem w duration

    // rozbicie time_point na datę i godzinę
    auto now  = floor<seconds>(system_clock::now());
    auto date = year_month_day{floor<days>(now)};
    auto time = hh_mm_ss{now - floor<days>(now)};
}