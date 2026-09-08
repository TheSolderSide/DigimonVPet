#pragma once
#include <Arduino.h>
#include <cmath>


#define N_EVOLUTIONS 1 //the maximal evolution options per digimon
#define N_DIGIMON 15 //the number of digimon in the game
#define DIGIMON_EGG 0
#define DIGIMON_BOTAMON 1
#define DIGIMON_KOROMON 2
#define DIGIMON_AGUMON 3
#define DIGIMON_BETAMON 4
#define DIGIMON_GREYMON 5
#define DIGIMON_TYRANOMON 6
#define DIGIMON_DEVIMON 7
#define DIGIMON_MERAMON 8
#define DIGIMON_AIRDRAMON 9
#define DIGIMON_SEADRAMON 10
#define DIGIMON_NUMEMON 11
#define DIGIMON_METALGREYMON 12
#define DIGIMON_MAMEMON 13
#define DIGIMON_MONZAEMON 14

#define TYPE_VACCINE 0
#define TYPE_DATA 1
#define TYPE_VIRUS 2
#define TYPE_FREE 3

#define STAGE_EGG 0
#define STAGE_BABY1 1
#define STAGE_BABY2 2
#define STAGE_ROOKIE 3
#define STAGE_ADULT 4
#define STAGE_PERFECT 5
#define STAGE_ULTIMATE 6
#define STAGE_SUPER_ULTIMATE 7

#define POOP_FREQUENCY_BABY1 60*3 //3 minutes
#define POOP_FREQUENCY_BABY2 60*30 //30 minutes
#define POOP_FREQUENCY_ROOKIE 60*60 // 1hour
#define POOP_FREQUENCY_ADULT 60*70 // 70 minutes
#define POOP_FREQUENCY_PERFECT 60*80
#define POOP_FREQUENCY_ULTIMATE 60*100

#define EVOLUTION_TIME_EGG 60*1 //egg takes 1 minute to hatch
#define EVOLUTION_TIME_BABY1 60*10 //Baby1->Baby2 takes 10 minutes
#define EVOLUTION_TIME_BABY2 60*60*6 // Baby2->rookie takes 6 hours
#define EVOLUTION_TIME_ROOKIE 60*60*24 // Rookie->adult takes 24 hours
#define EVOLUTION_TIME_ADULT 60*60*36 
#define EVOLUTION_TIME_PERFECT 60*60*48

#define STATE_EGG 0
#define STATE_AWAKE 1
#define STATE_TIRED 2
#define STATE_ASLEEP 3
#define STATE_SICK 4
#define STATE_INJURED 5
#define STATE_BATTLING 6
#define STATE_DEAD 7

struct DigimonProperties {
    char* digiName; //Name of the Digimon
    uint8_t stage; //baby rookie adult etc.
    uint16_t minWeight;
    uint8_t stomachCapacity;
    uint8_t maxEnergy;

    uint8_t sleepHour;//at what time the digimon begins sleeping (0-23)
    uint8_t wakeUpHour;//at what time the digimon wakes up(0-23)
    unsigned long poopTimeSec; //the time it takes to poop in seconds
    unsigned long evolutionTimeSec; //time it takes to evolve in seconds
    unsigned long feedTimeSec; //time it takes for hunger to increase by 1 in seconds
    uint8_t type; // Va Da Vi
    uint8_t og_slot; // slot for battles in og mode
    uint16_t evolutionOptions; // how many possible evolutions there are in the evolution data array in progmem
};

const DigimonProperties DIGIMON_DATA[N_DIGIMON] PROGMEM = {
    {"Egg",STAGE_EGG,0,0,0,0,0,POOP_FREQUENCY_ULTIMATE,EVOLUTION_TIME_EGG ,600,TYPE_DATA,0x03,1},
    {"Botamon",STAGE_BABY1,5 ,4 ,20,19,8,POOP_FREQUENCY_BABY1,EVOLUTION_TIME_BABY1 ,600,TYPE_DATA,0x03,1},
    {"Koromon",STAGE_BABY2,10,16,20,19,8,POOP_FREQUENCY_BABY2,EVOLUTION_TIME_BABY2,600,TYPE_DATA,0x03,1},
    {"Agumon",STAGE_ROOKIE,20,24,20,19,8,POOP_FREQUENCY_ROOKIE,EVOLUTION_TIME_ROOKIE,600,TYPE_DATA,0x03,0},
    {"Betamon",STAGE_ROOKIE,20,24,20,19,8,POOP_FREQUENCY_ROOKIE,EVOLUTION_TIME_ROOKIE,600,TYPE_VACCINE,0x03,0},
    {"Greymon",STAGE_ADULT,30,28,30,19,8,POOP_FREQUENCY_ADULT,EVOLUTION_TIME_ADULT,600,TYPE_DATA,0x03,0},
    {"Tyrannomon",STAGE_ADULT,20,28,30,19,8,POOP_FREQUENCY_ADULT,EVOLUTION_TIME_ADULT,600,TYPE_VIRUS,0x03,0},
    {"Devimon",STAGE_ADULT,40,32,40,19,8,POOP_FREQUENCY_ADULT,EVOLUTION_TIME_ADULT,600,TYPE_VIRUS,0x03,0},
    {"Meramon",STAGE_ADULT,30,32,30,19,8,POOP_FREQUENCY_ADULT,EVOLUTION_TIME_ADULT,600,TYPE_VACCINE,0x03,0},
    {"AirDramon",STAGE_ADULT,30,24,30,19,8,POOP_FREQUENCY_ADULT,EVOLUTION_TIME_ADULT,600,TYPE_DATA,0x03,0},
    {"Seadramon",STAGE_ADULT,20,28,20,19,8,POOP_FREQUENCY_ADULT,EVOLUTION_TIME_ADULT,600,TYPE_DATA,0x03,0},
    {"Numemon",STAGE_ADULT,10,16,10,19,8,POOP_FREQUENCY_ADULT,EVOLUTION_TIME_ADULT,600,TYPE_DATA,0x03,0},
    {"MetalGreymon",STAGE_PERFECT,40,36,40,19,8,POOP_FREQUENCY_PERFECT,EVOLUTION_TIME_PERFECT,600,TYPE_DATA,0x03,0},
    {"Mamemon",STAGE_PERFECT,5,32,100,19,8,POOP_FREQUENCY_PERFECT,EVOLUTION_TIME_PERFECT,600,TYPE_VACCINE,0x03,0},
    {"Monzaemon",STAGE_PERFECT,40,32,50,19,8,POOP_FREQUENCY_PERFECT,EVOLUTION_TIME_PERFECT,600,TYPE_VIRUS,0x03,0}
};


// Saved separately from the legacy fields. An unresolved episode is shared by
// hunger, strength and sleeping with the lights on.
struct CareTrackingState {
    uint32_t elapsedMs = 0;
    bool active = false;
    bool mistakeLogged = false;
    bool overfed = false;
    bool bedtimeHandled = false;
    bool wasInSleepWindow = false;
};

class Digimon{

    private:

        uint16_t digimonIndex=0;

        const DigimonProperties* properties = nullptr;

        //variables not to save
        boolean evolved=false;

        //variables to save
        uint8_t state=0; 
        uint16_t age=0;
        uint16_t weight=0;
        uint16_t feedCounter = 0;
        uint16_t careMistakes = 0;
        uint16_t trainingCounter = 0;
        uint8_t numberOfPoops = 0;
        uint8_t hunger = 0;
        uint8_t strength = 0;
        uint8_t energy = 0; // available battle energy
        uint8_t overfeedCounter = 0;
        uint8_t sleepDisturbancesCounter = 0;
        uint8_t sicknessCounter = 0;

        //uint16_t singleTotalBattleRecord
        //uint16_t tagTotalBattleRecord
        //uint16_t singleTotalBattleWins
        //uint16_t tagTotalBattleWins

        //timers
        unsigned long poopTimer = 0;
        unsigned long ageTimer = 0;
        unsigned long evolutionTimer = 0;
        unsigned long feedTimer = 0;

        //sleep related flags
        // default lights ON so newly hatched digimon don't immediately auto-sleep
        boolean lightsOn = true;
        boolean forcedAsleep = false;
        // avoid counting the same "lights kept on" care mistake multiple times per night
        boolean sleepCareMistakeLogged = false;
        // when sleep is locked (OFF), only sleep menu can be accessed
        boolean sleepLocked = false;

        CareTrackingState care;
        bool callAlertPending = false;
        bool inBedtime = false;
        bool fullNightCandidate = false;
        uint32_t nightSleepMs = 0;
        void updateTimers(unsigned long delta);
        bool needsCare();
        static uint8_t clampStat(int value) { return value < 0 ? 0 : (value > 10 ? 10 : value); }


    public:

        Digimon(uint16_t index){digimonIndex =index;};
        void loop(unsigned long delta);
        boolean isEvolved(){return evolved;};

        //setters
        void setProperties(const DigimonProperties* value){properties=value; setWeight(weight); setEnergy(energy);}
        void setDigimonIndex(uint16_t _digimonIndex){digimonIndex=_digimonIndex;};
        void setState(uint8_t _state){state=_state;}; 
        void setAge(uint16_t _age){age=_age;};
        void setWeight(uint16_t value){weight = properties && value < properties->minWeight ? properties->minWeight : value;}
        void setFeedCounter( uint16_t _feedCounter){feedCounter=_feedCounter;};
        void setCareMistakes(uint16_t _careMistakes){careMistakes=_careMistakes;};
        void setTrainingCounter( uint16_t _trainingCounter){trainingCounter=_trainingCounter;};
        void setPoopTimer(unsigned long _poopTimer){poopTimer=_poopTimer;};
        void setAgeTimer(unsigned long _ageTimer){ageTimer=_ageTimer;};
        void setEvolutionTimer(unsigned long _evolutionTimer){evolutionTimer=_evolutionTimer;};
        void setEvolved(boolean _evolved){evolved=_evolved;};
        void setNumberOfPoops(uint8_t _numberOfPoops){numberOfPoops=_numberOfPoops;};
        void setHunger(uint8_t value){hunger = value > getFoodCapacity() ? getFoodCapacity() : value;};
        void setStrength(uint8_t _strength){strength=clampStat(_strength);};
        void setEnergy(uint8_t value){energy = properties && value > properties->maxEnergy ? properties->maxEnergy : value;}
        void setOverfeedCounter(uint8_t _overfeedCounter){overfeedCounter=_overfeedCounter;};
        void setSleepDisturbancesCounter(uint8_t _sleepDisturbancesCounter){sleepDisturbancesCounter=_sleepDisturbancesCounter;};
        void setSicknessCounter(uint8_t _sicknessCounter){sicknessCounter=_sicknessCounter;};
        
        //getters
        
        uint16_t getDigimonIndex(){return digimonIndex;};
        const DigimonProperties* getProperties(){return properties;};
        uint8_t getState(){return state;}; 
        uint16_t getAge(){return age;};
        uint16_t getWeight(){return weight;};
        uint16_t getFeedCounter(){return feedCounter;};
        uint16_t getCareMistakes(){return careMistakes;};
        uint16_t getTrainingCounter(){return trainingCounter;};
        unsigned long getPoopTimer(){return poopTimer;};
        unsigned long getAgeTimer(){return ageTimer;};
        unsigned long getEvolutionTimer(){return evolutionTimer;};
        unsigned long getFeedTimer(){return feedTimer;};
        uint8_t getNumberOfPoops(){return numberOfPoops;};
        uint8_t getHunger(){return hunger;};
        uint8_t getStrength(){return strength;};
        uint8_t getEnergy(){return energy;};
        uint8_t getHungerHearts(){return std::round(4.0 * (hunger > 10 ? 10 : hunger) / 10.0);};
        uint8_t getStrengthHearts(){return std::round(4.0 * getStrength() / 10.0);};
        uint8_t getOverfeedCounter(){return overfeedCounter;};
        uint8_t getSleepDisturbancesCounter(){return sleepDisturbancesCounter;};
        uint8_t getSicknessCounter(){return sicknessCounter;};

        void printSerial();
        uint8_t getFoodCapacity(){ return properties && properties->stomachCapacity > 10 ? properties->stomachCapacity : 10; }
        void reduceHunger(int8_t amount){
            int value = (int)hunger - amount;
            hunger = value < 0 ? 0 : (value > getFoodCapacity() ? getFoodCapacity() : value);
            if(getHungerHearts() < 4) care.overfed = false;
        }
        void increaseHunger(int8_t amount){ reduceHunger(-amount); }
        void addWeight(int8_t w){weight += w;}
        void loseWeight(int8_t w){if (w > 0) setWeight(weight > w ? weight - w : 0);}
        void addStrength(int8_t s){strength = clampStat((int)strength + s);}
        void loseStrength(int8_t s){addStrength(-s);}

        bool feedMeal();
        bool feedProtein();
        void beginTraining();
        void finishTraining(bool won);
        bool disturbSleep();
        bool cure();
        void applyLights(bool on);
        void updateSleepSchedule(uint8_t hours, uint8_t minutes, bool clockChanged = false);
        void updateCare(unsigned long delta);
        bool isCallActive(){ return care.active && !care.mistakeLogged; }
        bool hasCallAlert(){ return callAlertPending && isCallActive(); }
        void acknowledgeCallAlert(){ callAlertPending = false; }
        CareTrackingState getCareTrackingState(){ return care; }
        void restoreCareTrackingState(const CareTrackingState& value){ care = value; }
        void setFeedTimer(unsigned long value){ feedTimer = value; }

        uint8_t getEnergyPercentage(){ return properties && properties->maxEnergy ? 100U * energy / properties->maxEnergy : 0; }
        void addEnergy(uint8_t amount){
            const unsigned int total = (unsigned int)energy + amount;
            energy = properties ? (total > properties->maxEnergy ? properties->maxEnergy : total) : 0;
        }
        bool spendEnergy(uint8_t cost){
            if (cost > energy) return false;
            energy -= cost;
            return true;
        }

        //sleep / lights control
        void setLightsOn(bool v){lightsOn = v;};
        bool isLightsOn(){return lightsOn;};
        void setForcedAsleep(bool v){forcedAsleep = v;};
        bool isForcedAsleep(){return forcedAsleep;};
        void setSleepCareMistakeLogged(bool v){ sleepCareMistakeLogged = v; };
        bool isSleepCareMistakeLogged(){ return sleepCareMistakeLogged; };
        void setSleepLocked(bool v){ sleepLocked = v; };
        bool isSleepLocked(){ return sleepLocked; };
};
