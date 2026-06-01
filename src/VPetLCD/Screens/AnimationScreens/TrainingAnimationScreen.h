#pragma once
#include <functional>
#include "../../VPetLCD.h"
#include "../../../GameLogic/Digimon.h"

class TrainingAnimationScreen : public VPetLCD::Screen {
  private:
    AbstractSpriteManager* spriteManager;
    Digimon* digimon;
    uint16_t digimonSpriteIndex;

    uint8_t rounds;
    uint8_t currentRound;
    uint8_t blockedCount;

    // -1 none, 0 bottom, 1 top
    int8_t playerChoicePos;
    int8_t opponentChoicePos;

    enum Mode { MODE_DEFEND=0, MODE_ATTACK=1 };
    Mode mode;

    // staging for simple in-screen animation
    uint8_t stage;
    unsigned long stageTimer;

    // callback when game ends
    std::function<void(void)> endCallback;

  public:
    TrainingAnimationScreen(AbstractSpriteManager* _spriteManager, uint16_t _digimonSpriteIndex, Digimon* _digimon, uint8_t _mode=MODE_DEFEND);
    void startGame();
    void chooseShieldTop();
    void chooseShieldBottom();
    void loop(unsigned long delta);
    void draw(VPetLCD* lcd) override;
    void setDigimonSpriteIndex(uint16_t _idx){ digimonSpriteIndex = _idx; };
    void setEndCallback(std::function<void(void)> cb){ endCallback = cb; };
};
