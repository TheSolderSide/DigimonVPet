#pragma once
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <algorithm>
using std::min;
using boolean = bool;
using byte = uint8_t;
#define PROGMEM
struct TestSerial {
    template<class T> void println(T) {}
    template<class... T> void printf(const char*, T...) {}
};
inline TestSerial Serial;
// Deterministic training opponent chooses bottom.
inline long random(long, long) { return 0; }
inline long playerRoll = 0, enemyRoll = 0;
inline unsigned battleRoll = 0;
inline long random(long upper) {
    return upper == 100 ? ((battleRoll++ % 2) == 0 ? playerRoll : enemyRoll) : 0;
}
inline unsigned long millis() { return 0; }
