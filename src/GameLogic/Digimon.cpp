#include "Digimon.h"

void Digimon::printSerial(){
  Serial.println(getDigimonIndex());
  Serial.println(getState());   
  Serial.println(getAge());
  Serial.println(getWeight());
  Serial.println(getFeedCounter());
  Serial.println(getCareMistakes());
  Serial.println(getTrainingCounter());
  Serial.println(getPoopTimer());
  Serial.println(getAgeTimer());
  Serial.println(getEvolutionTimer());
}

namespace {
constexpr unsigned long CARE_WINDOW_MS = 20UL * 60 * 1000;
bool dailyWindow(int now, int start, int end) {
    return (now - start + 1440) % 1440 < (end - start + 1440) % 1440;
}
}

bool Digimon::needsCare() {
    return state != STATE_EGG && state != STATE_DEAD &&
        (getHungerHearts() == 0 || getStrengthHearts() == 0 ||
         (state == STATE_ASLEEP && lightsOn));
}

void Digimon::updateCare(unsigned long delta) {
    if (!needsCare()) {
        care.active = false;
        care.mistakeLogged = false;
        care.elapsedMs = 0;
        callAlertPending = false;
        return;
    }
    if (!care.active) {
        care.active = true;
        care.elapsedMs = 0;
        callAlertPending = true;
    }
    if (!care.mistakeLogged) {
        const unsigned long remaining = CARE_WINDOW_MS - care.elapsedMs;
        care.elapsedMs += delta < remaining ? delta : remaining;
        if (care.elapsedMs == CARE_WINDOW_MS) {
            if (careMistakes < UINT16_MAX) ++careMistakes;
            care.mistakeLogged = true;
            callAlertPending = false;
        }
    }
}

bool Digimon::feedMeal() {
    if (state == STATE_EGG || state == STATE_DEAD || state == STATE_ASLEEP || hunger >= getFoodCapacity()) return false;
    increaseHunger(1);
    addWeight(1);
    if (feedCounter < UINT16_MAX) ++feedCounter;
    if (hunger == getFoodCapacity() && !care.overfed) {
        if (overfeedCounter < UINT8_MAX) ++overfeedCounter;
        care.overfed = true;
    }
    updateCare(0);
    return true;
}

bool Digimon::feedProtein() {
    if (state == STATE_EGG || state == STATE_DEAD || state == STATE_ASLEEP || strength >= 10) return false;
    addStrength(2);
    addWeight(2);
    addDigimonPower(2);
    updateCare(0);
    return true;
}

void Digimon::beginTraining() {
    if (trainingCounter < UINT16_MAX) ++trainingCounter;
}

void Digimon::finishTraining(bool won) {
    if (!won) return;
    addStrength(1);
    addDigimonPower(10);
    updateCare(0);
}

bool Digimon::disturbSleep() {
    if (state != STATE_ASLEEP) return false;
    if (sleepDisturbancesCounter < UINT8_MAX) ++sleepDisturbancesCounter;
    applyLights(true);
    return true;
}

void Digimon::applyLights(bool on) {
    if (state == STATE_EGG || state == STATE_DEAD) return;
    lightsOn = on;
    forcedAsleep = !on;
    state = on ? (inBedtime ? STATE_TIRED : STATE_AWAKE) : STATE_ASLEEP;
    // A deliberate wake stays awake until lights OFF or the next night's bedtime.
    if (care.wasInSleepWindow) care.bedtimeHandled = true;
    updateCare(0);
}

void Digimon::updateSleepSchedule(uint8_t hours, uint8_t minutes, bool clockChanged) {
    if (!properties || state == STATE_EGG || state == STATE_DEAD) return;
    const int now = hours * 60 + minutes;
    const int bedtime = properties->sleepHour * 60;
    const int wake = properties->wakeUpHour * 60;
    const bool sleepingHours = dailyWindow(now, bedtime, wake);
    inBedtime = dailyWindow(now, (bedtime + 1440 - 30) % 1440, wake);
    if (!sleepingHours) {
        const bool wakeNow = care.wasInSleepWindow || clockChanged;
        care.bedtimeHandled = false;
        if (wakeNow && state == STATE_ASLEEP) {
            lightsOn = true;
            forcedAsleep = false;
            state = STATE_AWAKE;
        }
    } else if (!care.bedtimeHandled &&
               (state == STATE_AWAKE || state == STATE_TIRED || state == STATE_ASLEEP)) {
        state = STATE_ASLEEP;
        care.bedtimeHandled = true;
    }
    care.wasInSleepWindow = sleepingHours;
    if (state == STATE_AWAKE || state == STATE_TIRED) state = inBedtime ? STATE_TIRED : STATE_AWAKE;
    updateCare(0);
}

void Digimon::loop(unsigned long delta) {
    if (!properties || state == STATE_DEAD) return;
    updateTimers(delta);
}

void Digimon::updateTimers(unsigned long delta) {
    ageTimer += delta;
    const unsigned long day = 24UL * 60 * 60 * 1000;
    if (ageTimer >= day) { age += ageTimer / day; ageTimer %= day; }
    evolutionTimer += delta;
    const unsigned long evolutionInterval = properties->evolutionTimeSec * 1000UL;
    if (evolutionInterval && evolutionTimer >= evolutionInterval) {
        evolved = true;
        evolutionTimer %= evolutionInterval;
    }
    if (state == STATE_EGG) return;

    // Sleep pauses depletion, but a pre-existing empty need remains unresolved.
    if (state == STATE_ASLEEP) { updateCare(delta); return; }
    poopTimer += delta;
    const unsigned long poopInterval = properties->poopTimeSec * 1000UL;
    if (poopInterval && poopTimer >= poopInterval) {
        const unsigned long piles = poopTimer / poopInterval;
        numberOfPoops = numberOfPoops + piles > 4 ? 4 : numberOfPoops + piles;
        weight = piles > weight ? 0 : weight - piles;
        poopTimer %= poopInterval;
    }
    const unsigned long feedInterval = properties->feedTimeSec * 1000UL;
    if (!feedInterval) { updateCare(delta); return; }
    feedTimer %= feedInterval;
    updateCare(0);
    // Split at depletion boundaries so long frames don't charge care time early.
    while (delta > 0) {
        const unsigned long remaining = feedInterval - feedTimer;
        const unsigned long step = delta < remaining ? delta : remaining;
        updateCare(step);
        feedTimer += step;
        delta -= step;
        if (feedTimer == feedInterval) {
            feedTimer = 0;
            reduceHunger(1);
            loseStrength(1);
            updateCare(0);
        }
    }
}
