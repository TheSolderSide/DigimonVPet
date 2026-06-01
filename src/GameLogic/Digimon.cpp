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

void Digimon::loop(unsigned long delta){
    updateTimers(delta);
}

void Digimon::updateTimers(unsigned long delta){
    poopTimer += delta;
    if(poopTimer > properties->poopTimeSec*1000){
        poopTimer %= properties->poopTimeSec*1000;
        numberOfPoops++;
        if(weight >0){
            loseWeight(1);
        }
    }

    ageTimer += delta;
    uint32_t day = 1000*60*60*24;
    if(ageTimer >= day){
        ageTimer %= day;
        age++;
    }

    evolutionTimer += delta;
    if(evolutionTimer >= properties->evolutionTimeSec*1000){
        Serial.println("evolving");
        evolved = true;
        setEvolutionTimer(0);
    }

    feedTimer += delta;
    if(feedTimer >= properties->feedTimeSec*1000*60*10){ //every 10 minutes, hunger increases by 1
        feedTimer =0;
        uint8_t hunger = getHunger();

        if(hunger < 0){
            reduceHunger(1);
        }

        if(hunger < 2){
            //send a warning some how
        }

        //if hunger at min and still not fed increase care mistakes
        if(hunger == 0){           
            careMistakes++;
        }
    }
}