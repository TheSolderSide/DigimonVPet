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
    // Fired once when the final result animation starts (true = win).
    std::function<void(bool)> resultCallback;
    // Fired once when an individual round's result is revealed.
    std::function<void(bool)> roundResultCallback;

  public:
    TrainingAnimationScreen(AbstractSpriteManager* _spriteManager, uint16_t _digimonSpriteIndex, Digimon* _digimon, uint8_t _mode=MODE_DEFEND);
    void startGame();
    // Display a result without starting/counting training or awarding stats.
    void showResult(bool won) { stage = won ? 2 : 3; stageTimer = 0; }
    void chooseShieldTop();
    void chooseShieldBottom();
    void loop(unsigned long delta);
    void draw(VPetLCD* lcd) override;
    void setDigimonSpriteIndex(uint16_t _idx){ digimonSpriteIndex = _idx; };
    void setEndCallback(std::function<void(void)> cb){ endCallback = cb; };
    void setResultCallback(std::function<void(bool)> cb){ resultCallback = cb; };
    void setRoundResultCallback(std::function<void(bool)> cb){ roundResultCallback = cb; };
};
