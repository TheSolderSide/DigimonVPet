#include "EvolutionHandler.h"

uint16_t EvolutionHandler::getEvolutionOption(Digimon currentDigimon){
    switch(currentDigimon.getDigimonIndex()){
        case DIGIMON_EGG:
            return DIGIMON_BOTAMON;
        case DIGIMON_BOTAMON:
            return DIGIMON_KOROMON;
        case DIGIMON_KOROMON:
            if(currentDigimon.getCareMistakes()>=3){
                return DIGIMON_BETAMON;
            }else{
                return DIGIMON_AGUMON;
            }
        case DIGIMON_AGUMON:
            if(currentDigimon.getCareMistakes() <= 3 && currentDigimon.getTrainingCounter() >= 32){
                return DIGIMON_GREYMON;
            }else if(currentDigimon.getCareMistakes() > 4 && 
            currentDigimon.getTrainingCounter() >= 5 && currentDigimon.getTrainingCounter() <= 15 && 
            currentDigimon.getOverfeedCounter() >= 3 && 
            currentDigimon.getSleepDisturbancesCounter() <= 4){
                return DIGIMON_TYRANOMON;
            } else if(currentDigimon.getCareMistakes() <= 3 && currentDigimon.getTrainingCounter() >= 31){
                return DIGIMON_DEVIMON;
            } else if (currentDigimon.getCareMistakes() <= 4 && 
            currentDigimon.getTrainingCounter() >= 16 &&
            currentDigimon.getOverfeedCounter() >= 3 &&
            currentDigimon.getSleepDisturbancesCounter() <= 6){
                return DIGIMON_MERAMON;
            }else {
                return DIGIMON_NUMEMON;
            }
        case DIGIMON_BETAMON:       
            if (currentDigimon.getCareMistakes() <= 3 && currentDigimon.getTrainingCounter() >= 48){
                return DIGIMON_DEVIMON;
            } else if (currentDigimon.getCareMistakes() <= 4 && currentDigimon.getTrainingCounter() <= 48){
                return DIGIMON_MERAMON;
            } else if (currentDigimon.getCareMistakes() <= 4 &&
            currentDigimon.getTrainingCounter() >= 8 && 
            currentDigimon.getTrainingCounter() <= 31 && 
            currentDigimon.getOverfeedCounter() <= 3 &&
            currentDigimon.getSleepDisturbancesCounter() >= 9){
                return DIGIMON_AIRDRAMON;
            } else if (currentDigimon.getCareMistakes() <= 4 &&
            currentDigimon.getTrainingCounter() >= 8 && 
            currentDigimon.getTrainingCounter() <= 31 && 
            currentDigimon.getOverfeedCounter() >= 4 &&
            currentDigimon.getSleepDisturbancesCounter() <= 8) {
                return DIGIMON_SEADRAMON;
            } else{
                return DIGIMON_NUMEMON;
            }       
        case DIGIMON_DEVIMON:
        case DIGIMON_GREYMON:
        case DIGIMON_AIRDRAMON:
            return DIGIMON_METALGREYMON;
        case DIGIMON_MERAMON:
        case DIGIMON_SEADRAMON:
        case DIGIMON_TYRANOMON:
            return DIGIMON_MAMEMON;
        case DIGIMON_NUMEMON:
            return DIGIMON_MONZAEMON;
        default:
            return currentDigimon.getDigimonIndex();
    }
}