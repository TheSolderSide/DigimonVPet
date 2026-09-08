#pragma once

#include <Arduino.h>

// One passive piezo buzzer; call update() regularly from the Arduino loop.
class SoundManager {
public:
    enum class Sound : uint8_t { Beep = 1, Alert = 2, Happy = 3 };

    explicit SoundManager(uint8_t pin, uint8_t channel = 2);
    bool begin();
    void update();
    void play(Sound sound);
    void playBeep() { play(Sound::Beep); }
    void playAlert() { play(Sound::Alert); }
    void playHappy() { play(Sound::Happy); }
    void stop();
    void setEnabled(bool enabled);
    bool isPlaying() const { return notes != nullptr; }

private:
    struct Note { uint16_t frequency; uint16_t durationMs; };
    static const Note beep[];
    static const Note alert[];
    static const Note happy[];
    void writeTone(uint16_t frequency);

    uint8_t pin;
    uint8_t channel;
    bool initialized = false;
    bool enabled = true;
    const Note* notes = nullptr;
    size_t noteCount = 0;
    size_t noteIndex = 0;
    uint32_t noteStartedMs = 0;
    Sound currentSound = Sound::Beep;
};
