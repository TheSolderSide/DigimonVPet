#include "SaveGameHandler.h"

namespace {
constexpr int EXTENSION_ADDRESS = 64;
constexpr uint32_t EXTENSION_MAGIC = 0x56504532;
struct SaveExtension {
    uint32_t magic;
    uint8_t version;
    uint8_t overfeeds;
    uint8_t disturbances;
    bool forcedAsleep;
    uint32_t evolutionTimer;
    uint32_t feedTimer;
    CareTrackingState care;
};
static_assert(EXTENSION_ADDRESS + sizeof(SaveExtension) <= EEPROM_SIZE, "Save exceeds EEPROM");
}

void SaveGameHandler::loadDigimon(Digimon* digimon) {
    digimon->setDigimonIndex(EEPROM.readUShort(ADRESS_DIGIMONINDEX));
    if (digimon->getDigimonIndex() >= N_DIGIMON) digimon->setDigimonIndex(DIGIMON_EGG);
    digimon->setProperties(&DIGIMON_DATA[digimon->getDigimonIndex()]);
    digimon->setState(EEPROM.readByte(ADDRESS_STATE));
    digimon->setAge(EEPROM.readUShort(ADDRESS_AGE));
    digimon->setWeight(EEPROM.readUShort(ADDRESS_WEIGHT));
    digimon->setFeedCounter(EEPROM.readUShort(ADDRESS_FEEDCOUNTER));
    digimon->setCareMistakes(EEPROM.readUShort(ADDRESS_CAREMISTAKES));
    digimon->setTrainingCounter(EEPROM.readUShort(ADDRESS_TRAININGCOUNTER));
    digimon->setPoopTimer(EEPROM.readULong(ADDRESS_POOPTIMER));
    digimon->setAgeTimer(EEPROM.readULong(ADDRESS_AGETIMER));
    digimon->setEvolutionTimer(EEPROM.readULong(ADDRESS_EVOLUTIONETIMER));

    digimon->setNumberOfPoops(EEPROM.readByte(ADDRESS_NUMBEROFPOOPS));
    digimon->setHunger(EEPROM.readByte(ADDRESS_HUNGER));
    digimon->setStrength(EEPROM.readByte(ADDRESS_STRENGTH));
    digimon->setEffort(EEPROM.readByte(ADDRESS_EFFORT));
    digimon->setDigimonPower(EEPROM.readByte(ADDRESS_DIGIMONPOWER));
    // load lights and sleep-flag
    digimon->setLightsOn(EEPROM.readByte(ADDRESS_LIGHTS));
    digimon->setSleepCareMistakeLogged(EEPROM.readByte(ADDRESS_SLEEP_LOGGED));
    SaveExtension extra{};
    EEPROM.get(EXTENSION_ADDRESS, extra);
    if (extra.magic == EXTENSION_MAGIC && extra.version == 1) {
        digimon->setOverfeedCounter(extra.overfeeds);
        digimon->setSleepDisturbancesCounter(extra.disturbances);
        digimon->setForcedAsleep(extra.forcedAsleep);
        digimon->setEvolutionTimer(extra.evolutionTimer);
        digimon->setFeedTimer(extra.feedTimer);
        digimon->restoreCareTrackingState(extra.care);
    } else {
        // Old evolution timer bytes overlap DP/feed bytes; they cannot be recovered.
        digimon->setEvolutionTimer(0);
        digimon->setFeedTimer(EEPROM.readULong(FEED_TIMER));
        digimon->setOverfeedCounter(0);
        digimon->setSleepDisturbancesCounter(0);
        digimon->setForcedAsleep(!digimon->isLightsOn());
        digimon->restoreCareTrackingState(CareTrackingState{});
    }

}

void SaveGameHandler::saveDigimon(Digimon* digimon) {
    EEPROM.put(ADRESS_DIGIMONINDEX,digimon->getDigimonIndex());
    EEPROM.put(ADDRESS_STATE,digimon->getState());
    EEPROM.put(ADDRESS_AGE,digimon->getAge());
    EEPROM.put(ADDRESS_WEIGHT,digimon->getWeight());
    EEPROM.put(ADDRESS_FEEDCOUNTER,digimon->getFeedCounter());
    EEPROM.put(ADDRESS_CAREMISTAKES,digimon->getCareMistakes());
    EEPROM.put(ADDRESS_TRAININGCOUNTER,digimon->getTrainingCounter());
    EEPROM.put(ADDRESS_POOPTIMER,digimon->getPoopTimer());
    EEPROM.put(ADDRESS_AGETIMER,digimon->getAgeTimer());
    EEPROM.put(ADDRESS_EVOLUTIONETIMER,digimon->getEvolutionTimer());
    EEPROM.put(ADDRESS_NUMBEROFPOOPS,digimon->getNumberOfPoops());
    EEPROM.put(ADDRESS_HUNGER,digimon->getHunger());
    EEPROM.put(ADDRESS_STRENGTH,digimon->getStrength());
    EEPROM.put(ADDRESS_EFFORT,digimon->getEffort());
    EEPROM.put(ADDRESS_DIGIMONPOWER,digimon->getDigimonPower());
    EEPROM.put(FEED_TIMER,digimon->getFeedTimer());
    // persist lights and sleep-flag
    EEPROM.put(ADDRESS_LIGHTS, digimon->isLightsOn());
    EEPROM.put(ADDRESS_SLEEP_LOGGED, digimon->isSleepCareMistakeLogged());
    // Keep the legacy layout readable and store new fields plus intact timers
    // outside its overlapping evolution/DP/feed addresses.
    SaveExtension extra{};
    extra.magic = EXTENSION_MAGIC;
    extra.version = 1;
    extra.overfeeds = digimon->getOverfeedCounter();
    extra.disturbances = digimon->getSleepDisturbancesCounter();
    extra.forcedAsleep = digimon->isForcedAsleep();
    extra.evolutionTimer = digimon->getEvolutionTimer();
    extra.feedTimer = digimon->getFeedTimer();
    extra.care = digimon->getCareTrackingState();
    EEPROM.put(EXTENSION_ADDRESS, extra);
    EEPROM.commit();
}