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
