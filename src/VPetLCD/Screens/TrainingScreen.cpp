#include "TrainingScreen.h"
#include "../ImageData/SymbolData.h"

V20::TrainingScreen::TrainingScreen(): SelectionScreen(true){
    addOption((char*)"ATK", SYMBOL_ATTACK);
    addOption((char*)"DEF", SYMBOL_DEFEND);
}
