# Czas w C++ — ze szczególnym uwzględnieniem nowoczesnej biblioteki `std::chrono`

---

## 1. Jak się drzewiej liczyło czas w C++?

Przed C++11 używano biblioteki `<ctime>` z języka C.

```cpp
#include <ctime>
```

Najważniejsze elementy:

- **`std::time_t`** — zwykle liczba sekund od 1970-01-01 UTC (standard nie gwarantuje, ale tak jest w praktyce; od C++20 `system_clock` formalnie używa Unix time).
- **`std::tm`** — rozbita data: `tm_year` (lata od **1900**), `tm_mon` (**0–11**), `tm_mday` (1–31), `tm_hour`, `tm_min`, `tm_sec`, `tm_wday`, `tm_yday`, `tm_isdst`.
- **`std::mktime`** — `tm` (czas lokalny) → `time_t`; normalizuje pola (np. `tm_mday = 32`).
- **`std::difftime(t1, t0)`** — różnica w sekundach jako `double`.
- **`std::clock()`** — **czas CPU procesu** (nie czas ścienny!) w tickach; dzielić przez `CLOCKS_PER_SEC`.
- **`std::timespec_get`** (C++17) — czas z dokładnością do nanosekund (`TIME_UTC`).
- **POSIX:** `clock_gettime(CLOCK_MONOTONIC / CLOCK_REALTIME / CLOCK_PROCESS_CPUTIME_ID)`, `gettimeofday` (przestarzałe).

Wady starego API: brak typów jednostek (sekundy to po prostu `long`), `localtime`/`gmtime` nie są wątkowo bezpieczne, brak sensownej obsługi ułamków sekund, brak stref poza „lokalną” i UTC.
Nie należy używać tego API chyba, że mamy naprawdę stary kod. 

---

## 2. `std::chrono` — Nowoczesny standart

```cpp
#include <chrono>
```

## Trzy najważniejsze pojęcia:

| Pojęcie | Znaczenie | Przykład |
|---|---|---|
| **duration** | *odcinek* czasu = liczba tików × okres tiku | `std::chrono::milliseconds{250}` |
| **time_point** | *punkt* na osi czasu = odcinek od epoki **konkretnego zegara** | `std::chrono::system_clock::now()` |
| **clock** | źródło `time_point`-ów: definiuje epokę, dokładność i `now()` | `std::chrono::steady_clock` |

Analogia: `duration` to „5 minut”, `time_point` to „godzina 14:05”, `clock` to zegarek, który mówi, która jest godzina.

Podstawowe reguły arytmetyki:

```
time_point - time_point  -> duration
time_point + duration    -> time_point
time_point - duration    -> time_point
duration   ± duration    -> duration
duration   * / skalar    -> duration
duration   / duration    -> liczba (rep wspólny)
time_point + time_point  -> BŁĄD (nie ma sensu)
```

Reguły arytmetyki w praktyce są pokazane w `arithmetic_chrono.cpp`

---

## 3. `std::chrono::duration`

### 3.1. Definicja

```cpp
template <class Rep, class Period = std::ratio<1>>
class duration;
```

- `Rep` — typ przechowujący liczbę tików (`int`, `long long`, `double`…).
- `Period` — `std::ratio` mówiący, ile sekund trwa jeden tik.
  
`std::ratio<Num, Den>` to ułamek wymierny znany w czasie kompilacji.
Zamiast ręcznie definiować Period można użyć std::micro, std::nano, std::kilo, std::mega, ... 

```cpp
std::chrono::duration<int>                     a{5};   // 5 s
std::chrono::duration<double, std::milli>      b{2.5}; // 2.5 ms
std::chrono::duration<long long, std::ratio<60>> c{3}; // 3 min
```

### 3.2. Predefiniowane typedefy

| Typ | Okres | Minimalna szerokość `rep` (wg standardu) |
|---|---|---|
| `nanoseconds` | 10⁻⁹ s | ≥ 64 bity |
| `microseconds` | 10⁻⁶ s | ≥ 55 bitów |
| `milliseconds` | 10⁻³ s | ≥ 45 bitów |
| `seconds` | 1 s | ≥ 35 bitów |
| `minutes` | 60 s | ≥ 29 bitów |
| `hours` | 3600 s | ≥ 23 bity |
| `days` *(C++20)* | 86 400 s | ≥ 25 bitów |
| `weeks` *(C++20)* | 7 dni | ≥ 22 bity |
| `months` *(C++20)* | 30,436875 dni (średnio) | ≥ 20 bitów |
| `years` *(C++20)* | 365,2425 dni (średnio) | ≥ 17 bitów |

> `months` i `years` to **średnie** długości (rok gregoriański), więc nadają się do arytmetyki kalendarzowej na typach `year_month_day`, ale nie do „dokładnych” obliczeń na sekundach.

### 3.3. Literały (C++14)

```cpp
using namespace std::chrono_literals;

auto d1 = 1h + 30min + 15s;     // typ: seconds (wspólny typ)
auto d2 = 250ms;                // milliseconds
auto d3 = 1.5s;                 // duration<double> (literał zmiennoprzecinkowy)
auto d4 = 10us + 5ns;           // nanoseconds
```

Dostępne sufiksy: `h`, `min`, `s`, `ms`, `us`, `ns`. W C++20 dodatkowo `d` (**dzień miesiąca**, typ `std::chrono::day` — nie `days`!) oraz `y` (rok, typ `year`).

### 3.4. Konwersje

**Niejawne** — tylko gdy bez straty informacji (z grubszej jednostki na drobniejszą lub gdy docelowy `rep` jest zmiennoprzecinkowy):

```cpp
std::chrono::milliseconds ms = 2s;             // OK: 2000 ms
std::chrono::duration<double> sec = 1500ms;    // OK: 1.5 s
// std::chrono::seconds s = 1500ms;            // BŁĄD KOMPILACJI (utrata dokładności)
```

**Jawne** — `duration_cast` (obcina w stronę zera!) oraz od C++17 `floor`, `ceil`, `round`:

```cpp
using namespace std::chrono;

auto t = 1999ms;
duration_cast<seconds>(t);   // 1 s   (obcięcie w stronę zera)
floor<seconds>(t);           // 1 s   (w dół)
ceil<seconds>(t);            // 2 s   (w górę)
round<seconds>(t);           // 2 s   (do najbliższej; przy remisie do parzystej)

duration_cast<seconds>(-1999ms);  // -1 s   (w stronę zera)
floor<seconds>(-1999ms);          // -2 s   (w dół)
```

Przykład z jednostką własną:

```cpp
using frames = std::chrono::duration<int, std::ratio<1, 60>>; // 60 klatek/s
auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(frames{1});
// 1/60 s = 16.67 ms  ->  ms.count() == 16  (obcięcie!)
```

### 3.5. Podstawowe operacje

```cpp
auto a = 90s;
a.count();                       // 90  (surowa liczba tików — "zdejmuje" typ)
a + 30s;  a - 10s;  a * 2;  a / 3;  a % 7s;
-a;  ++a;  a += 5s;
a == 1min + 30s;                 // true — porównania działają między różnymi jednostkami
a <=> 2min;                      // C++20: operator <=>

std::chrono::seconds::zero();    // 0 s
std::chrono::seconds::max();     // największa wartość rep
std::chrono::abs(-5s);           // C++17: 5 s
```

Przy działaniach na różnych jednostkach wynik ma `std::common_type` obu argumentów (np. `1h + 30min` → `minutes`, `1s + 1ms` → `milliseconds`).

### 3.6. Typy zmiennoprzecinkowe

```cpp
using fsec = std::chrono::duration<double>;           // sekundy jako double
using fms  = std::chrono::duration<double, std::milli>;

fms elapsed = t1 - t0;           // niejawna konwersja z dowolnego odcinka — bez obcinania
std::cout << elapsed.count() << " ms\n";
```

Trait `std::chrono::treat_as_floating_point<Rep>` decyduje, czy konwersje niejawne są dozwolone przy utracie dokładności.

### 3.7. Funkcje przyjmujące dowolny odcinek

```cpp
template <class Rep, class Period>
void wait_for_something(std::chrono::duration<Rep, Period> timeout);

wait_for_something(500ms);
wait_for_something(2s);
wait_for_something(std::chrono::duration<double>{0.25});
```

Dla interfejsów publicznych często lepiej przyjąć konkretny typ (np. `std::chrono::milliseconds`) — wtedy wywołanie `f(2s)` zadziała dzięki konwersji niejawnej, a wywołanie `f(1500us)` nie skompiluje się (co bywa pożądane, bo ujawnia utratę dokładności).

---

Ułamki są automatycznie skracane i obsługują arytmetykę w czasie kompilacji (`std::ratio_add`, `std::ratio_multiply`, …).
Przykłady w pliku `ratio.cpp`


## 4. `std::chrono::time_point`

### 4.1. Definicja

```cpp
template <class Clock, class Duration = typename Clock::duration>
class time_point;
```

Przechowuje `duration` od epoki zegara `Clock`.

```cpp
using namespace std::chrono;

time_point<system_clock> now = system_clock::now();        // domyślna dokładność zegara
time_point<system_clock, seconds> sec = floor<seconds>(now);
auto since_epoch = now.time_since_epoch();                 // duration
```

### 4.2. Operacje

```cpp
auto t0 = steady_clock::now();
// ...
auto t1 = steady_clock::now();

auto delta = t1 - t0;             // duration
auto later = t1 + 5s;             // time_point
bool before = t0 < t1;

auto coarse = time_point_cast<milliseconds>(t1);   // zmiana dokładności (obcięcie)
auto fl     = floor<seconds>(t1);                  // C++17
```

### 4.3. Obcinanie do dnia (C++20)

```cpp
auto tp        = system_clock::now();
auto day_start = floor<days>(tp);          // sys_days — północ UTC
auto tod       = tp - day_start;           // czas od północy (duration)
```

---

## 5. Zegary (clocks)

Każdy zegar udostępnia: `rep`, `period`, `duration`, `time_point`, `static constexpr bool is_steady` oraz `static time_point now()`.

### 5.1. Zegary C++11

| Zegar | Charakterystyka | Do czego |
|---|---|---|
| `system_clock` | Czas ścienny systemu (wall clock). **Może skakać** (NTP, ręczna zmiana, DST nie — jest w UTC). Obsługuje konwersję do `time_t`. | Znaczniki czasu, logi, daty |
| `steady_clock` | **Monotoniczny** — nigdy nie cofa się, tempo stałe. Epoka nieokreślona (nie da się przeliczyć na datę). | Mierzenie odcinków, timeouty, deadline'y |
| `high_resolution_clock` | Zegar o najkrótszym tiku. W praktyce **alias** `system_clock` (libstdc++) lub `steady_clock` (libc++, MSVC). | Najlepiej **unikać** — zachowanie niespójne |

```cpp
static_assert(std::chrono::steady_clock::is_steady);
// system_clock::is_steady == false
```

**Reguła kciuka:** mierzysz *jak długo* coś trwało → `steady_clock`. Potrzebujesz *która jest godzina/data* → `system_clock`.

Dokładność `now()` zależy od platformy (np. nanosekundy na Linuksie, 100 ns na Windows, mikrosekundy w części implementacji na macOS). Typ `period` mówi o jednostce przechowywania, a nie o realnej rozdzielczości zegara.

`system_clock` ↔ `time_t`:

```cpp
std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
auto tp       = std::chrono::system_clock::from_time_t(t);
```

### 5.2. Zegary C++20

| Zegar | Opis |
|---|---|
| `system_clock` | Od C++20 formalnie mierzy **Unix time** (sekundy od 1970-01-01 00:00:00 UTC, **bez sekund przestępnych**) |
| `utc_clock` | UTC **z** sekundami przestępnymi |
| `tai_clock` | Międzynarodowy Czas Atomowy; epoka 1958-01-01 |
| `gps_clock` | Czas GPS; epoka 1980-01-06 |
| `file_clock` | Zegar znaczników czasu plików (`std::filesystem::file_time_type`) |
| `local_t` | *Pseudo-zegar* — oznacza „czas lokalny bez przypisanej strefy” |

Aliasy typów punktów czasu:

```cpp
std::chrono::sys_time<D>   // time_point<system_clock, D>
std::chrono::sys_seconds   // sys_time<seconds>
std::chrono::sys_days      // sys_time<days>   — "data" jako liczba dni od epoki
std::chrono::local_time<D> // time_point<local_t, D>
std::chrono::local_seconds, local_days
std::chrono::utc_time<D>, tai_time<D>, gps_time<D>, file_time<D>
```

Konwersja między zegarami — `clock_cast`:

```cpp
using namespace std::chrono;
auto sys = system_clock::now();
auto utc = clock_cast<utc_clock>(sys);
auto tai = clock_cast<tai_clock>(sys);   // TAI − UTC = 37 s (od 2017 r.)
auto f   = clock_cast<file_clock>(sys);
```

Sekundy przestępne: `get_leap_second_info(utc_time)` zwraca informację, czy dany moment jest sekundą przestępną i ile ich było do tej pory.

---

## 6. Pomiar czasu wykonania (benchmarking)

```cpp
using clock = std::chrono::steady_clock;

const auto start = clock::now();
do_work();
const auto elapsed = clock::now() - start;
```

lub

```cpp
using clock = std::chrono::clock;

const auto start = clock::now();
do_work();
const auto elapsed = clock::now() - start;
```

Bardziej rozbudowany przykład w pliku `benchmarking.cpp`

### Pułapki mikrobenchmarków

- Pierwsze wywołanie bywa wolniejsze (zimny cache, ładowanie stron) — wykonuj **rozgrzewkę** i **wiele powtórzeń**, raportuj medianę/percentyle, nie pojedynczy pomiar.
- Sam `now()` kosztuje (zwykle kilkadziesiąt ns), więc mierzenie bardzo krótkich operacji wymaga pętli i dzielenia.
- Dla poważnych pomiarów użyj gotowego narzędzia (np. Google Benchmark, nanobench, Catch2 benchmark).
- `std::clock()` mierzy czas **CPU**, nie ścienny — dla programów wielowątkowych lub czekających na I/O wynik będzie inny niż `steady_clock`.

---

## 10. C++20: kalendarz

Nagłówek: `<chrono>`. Typy opisują **elementy daty** — każdy ma własny typ, dzięki czemu nie pomylisz dnia z miesiącem.

### 10.1. Typy

| Typ | Opis | Przykład |
|---|---|---|
| `day` | dzień miesiąca (1–31) | `15d` |
| `month` | miesiąc (1–12) | `std::chrono::March`, `month{3}` |
| `year` | rok | `2024y` |
| `weekday` | dzień tygodnia (0=niedziela … 6=sobota) | `std::chrono::Monday` |
| `weekday_indexed` | n-ty dzień tygodnia w miesiącu | `Monday[2]` (drugi poniedziałek) |
| `weekday_last` | ostatni dzień tygodnia w miesiącu | `Friday[std::chrono::last]` |
| `month_day` | dzień + miesiąc | `March/15` |
| `year_month` | rok + miesiąc | `2024y/March` |
| `year_month_day` | pełna data | `2024y/March/15` |
| `year_month_day_last` | ostatni dzień miesiąca | `2024y/February/last` |
| `year_month_weekday` | n-ty dzień tygodnia w miesiącu danego roku | `2024y/May/Monday[2]` |
| `hh_mm_ss<D>` | rozbity czas doby | `hh_mm_ss{14h + 30min}` |

Stałe: `January` … `December`, `Sunday` … `Saturday`, `last`.

Więcej przykładów składania dat w calendar.cpp

### 10.2. Walidacja

Typy kalendarzowe mogą reprezentować wartości **niepoprawne**; sprawdź przez `ok()`:

### 10.3. Konwersje: data ↔ `sys_days`

`sys_days` = `time_point<system_clock, days>` — „liczba dni od 1970-01-01”. To „pomost” do arytmetyki dni.

### 10.4. Dodawanie miesięcy i lat

Dla typów kalendarzowych `+ months` / `+ years` działa na polach, bez przechodzenia przez dni. Wynik może być **niepoprawną datą**:

```cpp
auto d = 2024y/January/31 + months{1};   // 2024-02-31 — nie istnieje
if (!d.ok()) {
    d = d.year()/d.month()/last;         // przycięcie do ostatniego dnia: 2024-02-29
}
```

(`sys_days + months` się nie skompiluje — `months` nie dzieli się bez reszty na `days`.)

### 10.5. Doba: `hh_mm_ss`

```cpp
using namespace std::chrono;

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
```

### 10.6. Przykład: ostatni piątek miesiąca

```cpp
using namespace std::chrono;

sys_days last_friday = sys_days{2024y/October/Friday[last]};
std::cout << year_month_day{last_friday} << '\n';   // 2024-10-25
```

---

## 11. C++20: strefy czasowe

Strefy czasowe opierają się na **bazie IANA tz database** (nazwy typu `"Europe/Warsaw"`, `"America/New_York"`). Biblioteka standardowa albo korzysta z danych systemowych, albo (zależnie od implementacji) z własnej kopii.

### 11.1. Podstawy

```cpp
using namespace std::chrono;

const time_zone* waw = locate_zone("Europe/Warsaw");
const time_zone* cur = current_zone();          // strefa systemowa

auto now = system_clock::now();

zoned_time zt{"Europe/Warsaw", now};            // ten sam moment, widok w strefie
std::cout << zt << '\n';                        // np. 2024-07-01 14:03:22.123456789 CEST

zoned_time ny{"America/New_York", zt};          // zmiana strefy tego samego momentu
std::cout << ny << '\n';                        // inna godzina, ten sam punkt na osi czasu

auto sys   = zt.get_sys_time();                 // sys_time (UTC)
auto local = zt.get_local_time();               // local_time (ścienny czas w strefie)
```

### 11.2. Informacje o strefie

```cpp
sys_info info = waw->get_info(system_clock::now());
info.offset;     // przesunięcie względem UTC (np. 7200s latem)
info.save;       // przesunięcie DST (np. 60min)
info.abbrev;     // "CEST"
info.begin, info.end;   // przedział obowiązywania tych ustawień
```

### 11.3. Czas lokalny → UTC: niejednoznaczności

Konwersja *czasu lokalnego* na moment w czasie bywa niejednoznaczna lub niemożliwa przez zmianę czasu:

```cpp
using namespace std::chrono;

// Warszawa, 2024-03-31: o 2:00 zegar przeskakuje na 3:00 -> godzina 2:30 NIE ISTNIEJE
try {
    zoned_time z{"Europe/Warsaw", local_days{2024y/March/31} + 2h + 30min};
} catch (const nonexistent_local_time& e) {
    // e.what() opisuje przerwę
}

// Warszawa, 2024-10-27: o 3:00 zegar cofa się na 2:00 -> godzina 2:30 zdarza się DWUKROTNIE
try {
    zoned_time z{"Europe/Warsaw", local_days{2024y/October/27} + 2h + 30min};
} catch (const ambiguous_local_time& e) { /* ... */ }

// Jawny wybór:
zoned_time early{"Europe/Warsaw",
                 local_days{2024y/October/27} + 2h + 30min,
                 choose::earliest};   // choose::latest dla późniejszego wystąpienia
```

### 11.4. Dobre praktyki dla stref

- **Przechowuj i przesyłaj czas jako UTC** (`sys_time` / Unix time / ISO 8601 z `Z`), a strefy stosuj dopiero przy wyświetlaniu lub interakcji z użytkownikiem.
- Do przyszłych zdarzeń „o 9:00 czasu lokalnego” zapisuj **czas lokalny + nazwę strefy**, nie UTC (reguły DST mogą się zmienić).
- Nie używaj `current_zone()` na serwerach do logiki biznesowej — zależy od konfiguracji maszyny.
- Baza stref bywa aktualizowana (`std::chrono::reload_tzdb()`, `get_tzdb()`), a nazwy stref mają aliasy (`link`).

---

## 12. Formatowanie i parsowanie (C++20)

### 12.1. Wypisywanie

Typy chrono mają `operator<<` oraz specjalizację `std::formatter`:

```cpp
#include <chrono>
#include <format>
#include <iostream>

using namespace std::chrono;
using namespace std::chrono_literals;

std::cout << 1500ms << '\n';              // 1500ms
std::cout << 2024y/March/15 << '\n';      // 2024-03-15
std::cout << system_clock::now() << '\n'; // 2024-03-15 12:34:56.123456789

auto now = floor<seconds>(system_clock::now());
std::string s1 = std::format("{}", now);                      // 2024-03-15 12:34:56
std::string s2 = std::format("{:%Y-%m-%d %H:%M:%S}", now);    // jawny format
std::string s3 = std::format("{:%F %T}", now);                // to samo (skróty)
std::string s4 = std::format("{:%A, %d %B %Y}", now);         // dzień tygodnia i miesiąc słownie (locale!)

std::string s5 = std::format("{}", 90s);                      // "90s"
std::string s6 = std::format("{:%H:%M:%S}", 3725s);           // "01:02:05"

zoned_time zt{"Europe/Warsaw", now};
std::string s7 = std::format("{:%F %T %Z (%z)}", zt);         // 2024-03-15 13:34:56 CET (+0100)
```

> Wypisanie `system_clock::now()` bez obcięcia daje dokładność zegara (np. nanosekundy). Użyj `floor<seconds>` / `floor<milliseconds>`, aby ją ograniczyć.

Często używane specyfikatory (jak w `strftime`, z rozszerzeniami):

| Spec. | Znaczenie | Spec. | Znaczenie |
|---|---|---|---|
| `%Y` | rok (4 cyfry) | `%H` | godzina 00–23 |
| `%m` | miesiąc 01–12 | `%M` | minuta |
| `%d` | dzień 01–31 | `%S` | sekundy (z ułamkiem, jeśli typ jest dokładniejszy) |
| `%F` | `%Y-%m-%d` | `%T` | `%H:%M:%S` |
| `%j` | dzień roku | `%Z` | skrót strefy |
| `%a` / `%A` | dzień tygodnia (skrót/pełny) | `%z` | przesunięcie `+hhmm` |
| `%b` / `%B` | miesiąc (skrót/pełny) | `%Q` / `%q` | liczba tików / jednostka (dla `duration`) |

### 12.2. Parsowanie

```cpp
#include <sstream>

using namespace std::chrono;

std::istringstream in{"2024-05-17 13:45:00"};
sys_seconds tp;
in >> std::chrono::parse("%F %T", tp);
if (in.fail()) {
    // błędny format
}

// z przesunięciem strefy (%z lub %Z):
std::istringstream in2{"2024-05-17 13:45:00 +0200"};
sys_seconds tp2;
in2 >> parse("%F %T %z", tp2);          // wynik przeliczony na UTC
```

Pod spodem działa `std::chrono::from_stream`. Dla dat samych (bez godziny) sparsuj do `year_month_day`.

---