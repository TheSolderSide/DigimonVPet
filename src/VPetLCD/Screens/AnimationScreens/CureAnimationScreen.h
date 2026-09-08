#pragma once
#include "TrainingAnimationScreen.h"

class CureAnimationScreen : public VPetLCD::Screen {
    AbstractSpriteManager* spriteManager;
    Digimon* digimon;
    TrainingAnimationScreen failureAnimation;
    std::function<void(bool)> endCallback;
    unsigned long elapsed = 0;
    bool treating = false;
    bool active = false;
public:
    CureAnimationScreen(AbstractSpriteManager* sprites, Digimon* pet);
    void start();
    void loop(unsigned long delta);
    void draw(VPetLCD* lcd) override;
    void setEndCallback(std::function<void(bool)> callback) { endCallback = callback; }
};
