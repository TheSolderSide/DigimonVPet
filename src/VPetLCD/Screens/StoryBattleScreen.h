#pragma once
#include "../VPetLCD.h"
#include "../../GameLogic/Digimon.h"
#include "../../GameLogic/BattleRules.h"
#include <functional>

class StoryBattleScreen : public VPetLCD::Screen {
    Digimon* pet;
    AbstractSpriteManager* sprites;
    enum Phase { READY, PLAYER_FIRE, ENEMY_HIT, ENEMY_FIRE, PLAYER_HIT, FINISH_FIRE, FINISH_HIT, RESULT, CHAMPION };
    Phase phase = READY;
    unsigned long timer = 0;
    uint8_t opponent = 3, playerSpecies = 0, round = 0, playerHits = 0, enemyHits = 0;
    int playerPower = 0, enemyPower = 0, outcome = 0;
    bool restSelected = false, playerHit = false, enemyHit = false, lane = false;
    bool refused = false;
    std::function<void()> save, leave;
    std::function<void(int)> feedback;
    void beginRound();
    void finish();
public:
    StoryBattleScreen(AbstractSpriteManager* manager, Digimon* digimon) : pet(digimon), sprites(manager) {}
    void setCallbacks(std::function<void()> persist, std::function<void()> exit, std::function<void(int)> sound) {
        save = persist; leave = exit; feedback = sound;
    }
    void open() {
        phase = pet->getBattleRecord().opponent == BattleRules::OPPONENTS ? CHAMPION : READY;
        timer = 0; restSelected = false; refused = false;
    }
    bool isFighting() const { return phase != READY && phase != RESULT; }
    void next() { if (phase == READY) { restSelected = !restSelected; refused = false; } }
    void confirm();
    void back() { if (!isFighting() && leave) leave(); }
    void loop(unsigned long delta);
    void draw(VPetLCD* lcd) override;
};

class BattleRecordScreen : public VPetLCD::Screen {
    Digimon* pet;
    uint8_t recordType;
public:
    // 0: match wins, 1: draws, 2: tournament wins.
    BattleRecordScreen(Digimon* digimon, uint8_t type) : pet(digimon), recordType(type) {}
    void draw(VPetLCD* lcd) override {
        const auto record = pet->getBattleRecord();
        lcd->drawCharArrayOnLCD((char*)(recordType == 2 ? "CHAMPS" : (recordType == 1 ? "DRAWS" : "WINS")), 2, 0, pixelColor);
        char count[6];
        snprintf(count, sizeof(count), "%u", (unsigned)(recordType == 2 ? record.tournamentWins :
                                                       (recordType == 1 ? record.draws : record.wins)));
        for (unsigned i = 0; count[i]; ++i)
            lcd->drawDigitOnLCD(count[i] - '0', 2 + i * 6, 8, pixelColor);
    }
};
