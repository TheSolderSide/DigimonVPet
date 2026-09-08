#include "SoundManager.h"
#include <esp_arduino_version.h>

// Frequency in Hz, duration in milliseconds; zero frequency is a rest.
const SoundManager::Note SoundManager::beep[] = {{2400, 45}};
const SoundManager::Note SoundManager::alert[] = {
    {1800, 120}, {0, 60}, {1100, 140}, {0, 60},
    {1800, 120}, {0, 60}, {1100, 180}
};
const SoundManager::Note SoundManager::happy[] = {
    {1568, 100}, {0, 25}, {1976, 100}, {0, 25},
    {2349, 100}, {0, 25}, {3136, 240}
};

SoundManager::SoundManager(uint8_t pin, uint8_t channel)
    : pin(pin), channel(channel) {}

bool SoundManager::begin() {
    if (initialized) return true;
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    initialized = ledcAttachChannel(pin, 2400, 10, channel);
#else
    initialized = ledcSetup(channel, 2400, 10) != 0;
    if (initialized) ledcAttachPin(pin, channel);
#endif
    if (initialized) writeTone(0);
    return initialized;
}

void SoundManager::writeTone(uint16_t frequency) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWriteTone(pin, frequency);
#else
    ledcWriteTone(channel, frequency);
#endif
}

void SoundManager::play(Sound sound) {
    Serial.println("Buzzer play entered");
    if (!initialized || !enabled) return;
    Serial.println("Buzzer initialization happened and enabled");
    // Button feedback must not cut off an alert or celebration.
    if (sound == Sound::Beep && isPlaying() && currentSound != Sound::Beep) return;
    Serial.println("free to play sound");

    switch (sound) {
    case Sound::Beep:
        notes = beep;
        noteCount = sizeof(beep) / sizeof(beep[0]);
        break;
    case Sound::Alert:
        notes = alert;
        noteCount = sizeof(alert) / sizeof(alert[0]);
        break;
    case Sound::Happy:
        notes = happy;
        noteCount = sizeof(happy) / sizeof(happy[0]);
        break;
    default:
        return;
    }
    currentSound = sound;
    noteIndex = 0;
    noteStartedMs = millis();
    writeTone(notes[0].frequency);
}

void SoundManager::update() {
    if (!isPlaying()) return;
    const uint32_t now = millis();
    // Unsigned subtraction also handles millis() wrapping around.
    while (now - noteStartedMs >= notes[noteIndex].durationMs) {
        noteStartedMs += notes[noteIndex].durationMs;
        if (++noteIndex == noteCount) {
            stop();
            return;
        }
        writeTone(notes[noteIndex].frequency);
    }
}

void SoundManager::stop() {
    if (initialized) writeTone(0);
    notes = nullptr;
    noteCount = 0;
    noteIndex = 0;
}

void SoundManager::setEnabled(bool value) {
    enabled = value;
    if (!enabled) stop();
}
