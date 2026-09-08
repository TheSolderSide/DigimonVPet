#include <cassert>
#include "../../src/VPetLCD/Screens/AnimationScreens/CureAnimationScreen.h"
#include <iostream>
#include "../../src/GameLogic/Digimon.h"
#include "../../src/GameLogic/EvolutionHandler.h"
#include "../../src/SaveGame/SaveGameHandler.h"
#include "../../src/VPetLCD/Screens/AnimationScreens/TrainingAnimationScreen.h"

// Training tests execute its real input/round/result logic without a display.
void VPetLCD::draw16BitArray(const uint16_t*, int16_t, int16_t, boolean, uint16_t) {}
void VPetLCD::drawCharArrayOnLCD(char*, int16_t, int16_t, uint16_t) {}
void VPetLCD::drawSymbol(uint16_t, int16_t, int16_t, boolean, uint16_t) {}

void VPetLCD::drawPixelOnLCD(int16_t, int16_t, uint16_t) {}

constexpr unsigned long minute = 60000;
Digimon pet() {
    Digimon d(DIGIMON_AGUMON);
    d.setProperties(&DIGIMON_DATA[DIGIMON_AGUMON]);
    d.setState(STATE_AWAKE);
    d.setHunger(10);
    d.setStrength(10);
    return d;
}

void careEpisodes() {
    auto d = pet();
    d.setHunger(0);
    d.updateCare(0);
    assert(d.isCallActive() && d.hasCallAlert());
    d.acknowledgeCallAlert();
    assert(!d.hasCallAlert());
    d.updateCare(20 * minute - 1);
    assert(d.getCareMistakes() == 0 && d.isCallActive());
    d.updateCare(1);
    assert(d.getCareMistakes() == 1 && !d.isCallActive());
    d.setStrength(0);
    d.updateCare(4 * 60 * minute);
    assert(d.getCareMistakes() == 1);
    d.feedMeal(); // Still zero visible hearts after one meal.
    d.feedMeal();
    d.updateCare(20 * minute);
    assert(d.getCareMistakes() == 1); // Strength still unresolved.
    d.feedProtein();
    assert(!d.isCallActive());
    d.setStrength(0);
    d.updateCare(0);
    assert(d.isCallActive());
    d.updateCare(20 * minute);
    assert(d.getCareMistakes() == 2);

    auto both = pet();
    both.setHunger(0); both.setStrength(0);
    both.updateSleepSchedule(19, 0);
    both.updateCare(20 * minute);
    assert(both.getCareMistakes() == 1);
    both.applyLights(false);
    both.updateCare(120 * minute);
    assert(both.getCareMistakes() == 1);

    auto timely = pet();
    timely.setHunger(0); timely.updateCare(19 * minute);
    timely.feedMeal(); timely.feedMeal();
    timely.updateCare(20 * minute);
    assert(timely.getCareMistakes() == 0);

    auto egg = pet(); egg.setState(STATE_EGG); egg.setHunger(0);
    egg.updateCare(60 * minute);
    assert(egg.getCareMistakes() == 0 && !egg.isCallActive());
}

void feedingAndDecay() {
    auto d = pet();
    while (d.feedMeal()) {}
    assert(d.getHunger() == d.getFoodCapacity());
    assert(d.getHungerHearts() == 4 && d.getOverfeedCounter() == 1);
    const auto weight = d.getWeight();
    for (int i = 0; i < 10; ++i) assert(!d.feedMeal());
    assert(d.getWeight() == weight && d.getOverfeedCounter() == 1);
    d.reduceHunger(1); assert(d.feedMeal());
    assert(d.getOverfeedCounter() == 1); // No lost heart yet.
    d.reduceHunger(d.getHunger() - 8);
    assert(d.getHungerHearts() == 3);
    while (d.feedMeal()) {}
    assert(d.getOverfeedCounter() == 2);
    d.addStrength(100); assert(d.getStrengthHearts() == 4);
    assert(!d.feedProtein());
    d.loseStrength(100); assert(d.getStrength() == 0);

    auto decay = pet();
    decay.setHunger(2); decay.setStrength(2);
    decay.loop(10 * minute);
    assert(decay.getHungerHearts() == 0 && decay.getStrengthHearts() == 0);
    assert(decay.getCareMistakes() == 0 && decay.isCallActive());
    decay.loop(20 * minute - 1); assert(decay.getCareMistakes() == 0);
    decay.loop(1); assert(decay.getCareMistakes() == 1);
    decay.loop(4 * 60 * minute); assert(decay.getCareMistakes() == 1);
    assert(decay.getNumberOfPoops() <= 8);

    auto sleeping = pet(); sleeping.setHunger(0); sleeping.applyLights(false);
    sleeping.loop(20 * minute);
    assert(sleeping.getHunger() == 0 && sleeping.getCareMistakes() == 1);
}

void sleepSchedule() {
    auto d = pet();
    d.updateSleepSchedule(18, 29); assert(d.getState() == STATE_AWAKE);
    d.updateSleepSchedule(18, 30); assert(d.getState() == STATE_TIRED);
    d.updateSleepSchedule(19, 0);
    assert(d.getState() == STATE_ASLEEP && d.isLightsOn() && d.isCallActive());
    d.updateCare(20 * minute - 1); assert(d.getCareMistakes() == 0);
    d.updateCare(1); assert(d.getCareMistakes() == 1);
    d.applyLights(false);
    d.updateSleepSchedule(7, 59); assert(d.getState() == STATE_ASLEEP);
    d.updateSleepSchedule(8, 0);
    assert(d.getState() == STATE_AWAKE && d.isLightsOn() && !d.isForcedAsleep());
    assert(d.getSleepDisturbancesCounter() == 0);
    d.updateSleepSchedule(19, 0);
    assert(d.disturbSleep()); assert(!d.disturbSleep());
    d.updateSleepSchedule(19, 1);
    assert(d.getState() == STATE_TIRED && d.getSleepDisturbancesCounter() == 1);
    d.applyLights(false); assert(d.disturbSleep());
    assert(d.getSleepDisturbancesCounter() == 2);
    d.applyLights(false); d.updateSleepSchedule(12, 0, true);
    assert(d.getState() == STATE_AWAKE && d.isLightsOn());
    assert(d.getSleepDisturbancesCounter() == 2);
    // Clock changes to daytime also wake a manually sleeping daytime pet.
    d.applyLights(false); d.updateSleepSchedule(13, 0, true);
    assert(d.getState() == STATE_AWAKE && d.isLightsOn());
    auto p = DIGIMON_DATA[DIGIMON_AGUMON]; p.sleepHour = 2; p.wakeUpHour = 10;
    auto custom = pet(); custom.setProperties(&p);
    custom.updateSleepSchedule(2, 0); custom.applyLights(false);
    custom.updateSleepSchedule(9, 59); assert(custom.getState() == STATE_ASLEEP);
    custom.updateSleepSchedule(10, 0); assert(custom.getState() == STATE_AWAKE);
}

void training() {
    auto d = pet();
    TrainingAnimationScreen game(nullptr, DIGIMON_AGUMON, &d, 0);
    game.startGame(); // Cancel immediately: version 1 still counts.
    assert(d.getTrainingCounter() == 1);
    game.startGame();
    for (int i = 0; i < 5; ++i) { game.chooseShieldTop(); game.loop(1301); }
    assert(d.getTrainingCounter() == 2);
    game.startGame();
    for (int i = 0; i < 4; ++i) { game.chooseShieldBottom(); game.loop(1301); }
    assert(d.getTrainingCounter() == 3);
    game.chooseShieldBottom(); game.loop(1301);
    assert(d.getTrainingCounter() == 3);
    game.loop(10000); game.loop(10000);
    assert(d.getTrainingCounter() == 3);
}

void savesAndEvolution() {
    auto d = pet();
    while (d.feedMeal()) {}
    SaveGameHandler saves;
    saves.saveDigimon(&d);
    Digimon full(DIGIMON_EGG); saves.loadDigimon(&full);
    full.reduceHunger(1); assert(full.feedMeal());
    assert(full.getOverfeedCounter() == 1); // Saved latch prevents recounting.
    d.setHunger(0); d.updateCare(19 * minute);
    d.updateSleepSchedule(19, 0); d.disturbSleep();
    d.setEvolutionTimer(1234567); d.setFeedTimer(54321);
    saves.saveDigimon(&d);
    Digimon loaded(DIGIMON_EGG); saves.loadDigimon(&loaded);
    assert(loaded.getOverfeedCounter() == 1 && loaded.getSleepDisturbancesCounter() == 1);
    assert(loaded.getEvolutionTimer() == 1234567 && loaded.getFeedTimer() == 54321);
    loaded.updateCare(minute);
    assert(loaded.getCareMistakes() == 1 && !loaded.isCallActive());
    saves.saveDigimon(&loaded); saves.loadDigimon(&loaded);
    loaded.updateCare(60 * minute); assert(loaded.getCareMistakes() == 1);
    // Actual accumulated care affects the existing evolution selector.
    EvolutionHandler evolution;
    auto baby = pet(); baby.setDigimonIndex(DIGIMON_KOROMON);
    assert(evolution.getEvolutionOption(baby) == DIGIMON_AGUMON);
    for (int i = 0; i < 3; ++i) {
        baby.setHunger(0); baby.updateCare(20 * minute);
        baby.feedMeal(); baby.feedMeal();
    }
    assert(evolution.getEvolutionOption(baby) == DIGIMON_BETAMON);
}

void curing() {
    auto d = pet();
    CureAnimationScreen screen(nullptr, &d);
    int completions = 0;
    screen.setEndCallback([&](bool treated) {
        ++completions;
        if (treated) assert(d.cure());
    });
    assert(!d.cure());
    screen.start(); screen.loop(1801); screen.loop(5000);
    assert(completions == 1 && d.getState() == STATE_AWAKE);
    assert(d.getTrainingCounter() == 0 && d.getStrength() == 10);
    d.setState(STATE_SICK);
    screen.start(); screen.loop(2499);
    assert(d.getState() == STATE_SICK);
    screen.loop(1); screen.loop(5000);
    assert(completions == 2 && d.getState() == STATE_AWAKE);
    assert(d.getTrainingCounter() == 0 && d.getCareMistakes() == 0);
    d.setState(STATE_SICK); d.setLightsOn(false);
    assert(d.cure() && d.getState() == STATE_ASLEEP);
}

void poopSickness() {
    auto d = pet();
    const unsigned long interval = d.getProperties()->poopTimeSec * 1000UL;
    d.loop(7 * interval);
    assert(d.getNumberOfPoops() == 7 && d.getState() == STATE_AWAKE);
    d.loop(interval);
    assert(d.getNumberOfPoops() == 8 && d.getState() == STATE_SICK);
    const auto mistakes = d.getCareMistakes();
    d.loop(interval);
    assert(d.getNumberOfPoops() == 8 && d.getCareMistakes() == mistakes);
    d.setNumberOfPoops(0);
    assert(d.getState() == STATE_SICK); // Cleaning does not replace medicine.
    assert(d.cure());
    d.loop(7 * interval);
    assert(d.getNumberOfPoops() == 7 && d.getState() != STATE_SICK);
    d.setNumberOfPoops(0);
    d.loop(interval);
    assert(d.getNumberOfPoops() == 1 && d.getState() != STATE_SICK);
    auto delayed = pet();
    delayed.loop(9 * interval);
    assert(delayed.getNumberOfPoops() == 8 && delayed.getState() == STATE_SICK);
}

void bootSaveHandling() {
    SaveGameHandler saves;
    auto d = pet();
    EEPROM.bytes.fill(0xFF);
    assert(!saves.loadDigimon(&d));
    assert(d.getDigimonIndex() == DIGIMON_AGUMON);
    d.setCareMistakes(7); d.setTrainingCounter(9); d.setNumberOfPoops(6);
    saves.saveDigimon(&d);
    Digimon restored(DIGIMON_EGG);
    assert(saves.loadDigimon(&restored));
    assert(restored.getDigimonIndex() == DIGIMON_AGUMON);
    assert(restored.getCareMistakes() == 7 && restored.getTrainingCounter() == 9);
    saves.resetDigimon(&restored);
    assert(restored.getState() == STATE_EGG && restored.isLightsOn());
    assert(restored.getCareMistakes() == 0 && restored.getTrainingCounter() == 0);
    assert(restored.getNumberOfPoops() == 0 && restored.getEvolutionTimer() == 0);
    assert(saves.loadDigimon(&d));
    assert(d.getDigimonIndex() == DIGIMON_EGG && d.getCareMistakes() == 0);
    assert(EEPROM.bytes[255] == 0xFF);
}

int main() {
    bootSaveHandling(); poopSickness(); curing(); careEpisodes(); feedingAndDecay(); sleepSchedule(); training(); savesAndEvolution();
    std::cout << "Care, feeding, sleep, training, persistence and evolution passed (Version 1)\n";
}
