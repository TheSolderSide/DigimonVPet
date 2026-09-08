#pragma once
#include <array>
#include <cstring>
#include <cstdint>
struct TestEEPROM {
    std::array<uint8_t, 256> bytes{};
    void begin(int) {}
    void commit() {}
    template<class T> void put(int address, const T& value) {
        std::memcpy(bytes.data() + address, &value, sizeof(T));
    }
    template<class T> void get(int address, T& value) {
        std::memcpy(&value, bytes.data() + address, sizeof(T));
    }
    uint8_t readByte(int address) { return bytes[address]; }
    uint16_t readUShort(int address) { uint16_t v; get(address, v); return v; }
    uint32_t readULong(int address) { uint32_t v; get(address, v); return v; }
};
inline TestEEPROM EEPROM;
