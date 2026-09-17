#include "StoryBattleScreen.h"
#include "../ImageData/SymbolData.h"
#include "../ImageData/DigimonSprites.h"

void StoryBattleScreen::confirm() {
    if (phase == RESULT) { open(); return; }
    if (phase != READY) return;
    if (restSelected) { if (leave) leave(); return; }
    auto record = pet->getBattleRecord();
    if (record.opponent >= BattleRules::OPPONENTS) { if (leave) leave(); return; }
    if ((pet->getState() != STATE_AWAKE && pet->getState() != STATE_TIRED) ||
        !pet->isLightsOn() || pet->getEnergy() < BattleRules::ENERGY_COST) {
        refused = true;
        if (feedback) feedback(-1);
        return;
    }
    playerPower = BattleRules::power(pet->getEnergyPercentage(), record.trainingWins,
                                    pet->getHungerHearts(), pet->getStrengthHearts());
    enemyPower = BattleRules::power(50 + record.opponent * 3, record.opponent, 4, 4);
    opponent = DIGIMON_AGUMON + record.opponent;
    playerSpecies = pet->getDigimonIndex();
    pet->spendEnergy(BattleRules::ENERGY_COST);
    if (save) save();
    playerHits = enemyHits = round = 0;
    beginRound();
}

void StoryBattleScreen::beginRound() {
    ++round;
    playerHit = random(100) < BattleRules::hitChance(playerPower, enemyPower);
    enemyHit = random(100) < BattleRules::hitChance(enemyPower, playerPower);
    lane = random(2);
    phase = PLAYER_FIRE;
    timer = 0;
}

void StoryBattleScreen::finish() {
    outcome = BattleRules::result(playerHits, enemyHits);
    pet->recordBattle(outcome);
    // Roll once per completed loss, before saving. Preserve existing illness,
    // injury or death if care changed the pet's health during the match.
    if (outcome < 0 && (pet->getState() == STATE_AWAKE ||
                       pet->getState() == STATE_TIRED || pet->getState() == STATE_ASLEEP) &&
        random(100) < BattleRules::LOSS_SICKNESS_PERCENT) {
        pet->setState(STATE_SICK);
    }
    if (save) save();
    if (feedback) feedback(outcome);
    phase = outcome == 0 ? RESULT : FINISH_FIRE;
    timer = 0;
}

void StoryBattleScreen::loop(unsigned long delta) {
    if (!isFighting()) return;
    timer += delta;
    if (phase == CHAMPION) {
        if (timer >= 3000) {
            pet->restartTournament();
            if (save) save();
            open();
        }
        return;
    }
    const unsigned long duration = (phase == PLAYER_FIRE || phase == ENEMY_FIRE || phase == FINISH_FIRE) ? 700 : 600;
    if (timer < duration) return;
    timer = 0;
    switch (phase) {
    case PLAYER_FIRE: phase = ENEMY_HIT; if (playerHit) ++playerHits; break;
    case ENEMY_HIT: phase = ENEMY_FIRE; lane = random(2); break;
    case ENEMY_FIRE: phase = PLAYER_HIT; if (enemyHit) ++enemyHits; break;
    case PLAYER_HIT: if (round == BattleRules::ROUNDS) finish(); else beginRound(); break;
    case FINISH_FIRE: phase = FINISH_HIT; break;
    case FINISH_HIT:
        phase = outcome > 0 && pet->getBattleRecord().opponent == BattleRules::OPPONENTS ? CHAMPION : RESULT;
        break;
    default: break;
    }
}

void StoryBattleScreen::draw(VPetLCD* lcd) {
    // All coordinates fit the actual 40x16 LCD. Training assets are 16x16 and 8x8.
    if (phase == CHAMPION) {
        // Alternate the title with a happy pose six times over three seconds.
        if ((timer / 250) % 2 == 0) {
            lcd->drawCharArrayOnLCD((char*)"CHAMP", 6, 1, pixelColor);
            lcd->drawCharArrayOnLCD((char*)"WIN", 12, 9, pixelColor);
        } else {
            lcd->draw16BitArray(sprites->getDigimonSprite(pet->getDigimonIndex(),
                SPRITE_DIGIMON_HAPPY), 12, 0, true, pixelColor);
            lcd->drawSymbol(SYMBOL_SUCCESS, 1, 4, false, pixelColor);
            lcd->drawSymbol(SYMBOL_SUCCESS, 31, 4, false, pixelColor);
        }
        return;
    }
    if (phase == READY) {
        const auto record = pet->getBattleRecord();
        if (record.opponent >= BattleRules::OPPONENTS) {
            lcd->drawCharArrayOnLCD((char*)"CHAMP", 2, 0, pixelColor);
            lcd->drawCharArrayOnLCD((char*)"HOME", 2, 8, pixelColor);
        } else if (refused) {
            lcd->drawCharArrayOnLCD((char*)"REST", 2, 0, pixelColor);
            lcd->drawCharArrayOnLCD((char*)"FIRST", 2, 8, pixelColor);
        } else {
            // Opponent preview alternates with the tournament match number.
            if ((millis() / 1600) % 2 == 0) {
                lcd->draw16BitArray(sprites->getDigimonSprite(DIGIMON_AGUMON + record.opponent,
                    SPRITE_DIGIMON_WALK_0), 24, 0, false, pixelColor);
            } else {
                lcd->drawSmallIntegerOnLCD(record.opponent + 1, 28, 0, pixelColor);
            }
            // Two visible rows like the sleep menu, with a compact cursor so
            // REST and the full 16-pixel opponent fit beside each other.
            lcd->drawCharArrayOnLCD((char*)"GO", 4, 1, pixelColor);
            lcd->drawCharArrayOnLCD((char*)"REST", 4, 9, pixelColor);
            const int arrowY = (restSelected ? 8 : 0) + 2;
            for (int y = 0; y < 5; ++y) {
                const int width = y <= 2 ? y + 1 : 5 - y;
                for (int x = 0; x < width; ++x)
                    lcd->drawPixelOnLCD(x, arrowY + y, pixelColor);
            }
        }
        return;
    }
    if (phase == RESULT) {
        lcd->drawCharArrayOnLCD((char*)(outcome > 0 ? "WIN" : (outcome < 0 ? "LOSE" : "DRAW")), 2, 0, pixelColor);
        lcd->drawSmallIntegerOnLCD(playerHits, 8, 9, pixelColor);
        lcd->drawSmallIntegerOnLCD(enemyHits, 25, 9, pixelColor);
        return;
    }
    const bool finisher = phase == FINISH_FIRE || phase == FINISH_HIT;
    const bool firing = phase == PLAYER_FIRE || phase == ENEMY_FIRE || phase == FINISH_FIRE;
    const bool showPlayer = phase == PLAYER_FIRE || phase == PLAYER_HIT ||
        (phase == FINISH_FIRE && outcome > 0) || (phase == FINISH_HIT && outcome < 0);
    const bool hit = finisher || (showPlayer ? enemyHit : playerHit);
    int pose = firing && timer < 400 ? SPRITE_DIGIMON_ATTACK_1 : SPRITE_DIGIMON_WALK_0;
    if (!firing && hit && timer >= 300) pose = SPRITE_DIGIMON_ANGRY_1;
    lcd->draw16BitArray(sprites->getDigimonSprite(showPlayer ? playerSpecies : opponent, pose),
                        showPlayer ? 0 : 24, 0, showPlayer, pixelColor);
    if (firing) {
        // Missile leaves the LCD, then the view cuts to the defender on the opposite side.
        const int distance = timer * 24 / 700;
        const int x = showPlayer ? 16 + distance : 16 - distance;
        lcd->drawSymbol(SYMBOL_ATTACK, x, lane ? 0 : 8, showPlayer, pixelColor);
        if (finisher) lcd->drawSymbol(SYMBOL_ATTACK, x, lane ? 8 : 0, showPlayer, pixelColor);
    } else {
        // Every defender raises a shield. A normal hit gets through because
        // the shield covers the other lane; the double finisher covers both.
        const bool shieldTop = hit ? !lane : lane;
        if (timer < 300) {
            // Blocked missiles stop at the shield's outer edge. Show the
            // shield throughout the approach, then hold it after impact.
            const int travel = hit ? 16 : 8;
            const int distance = timer * travel / 300;
            const int incomingX = showPlayer ? 32 - distance : distance;
            lcd->drawSymbol(SYMBOL_ATTACK, incomingX, lane ? 0 : 8, !showPlayer, pixelColor);
            if (finisher) lcd->drawSymbol(SYMBOL_ATTACK, incomingX, lane ? 8 : 0, !showPlayer, pixelColor);
            lcd->drawSymbol(SYMBOL_DEFEND, 16, shieldTop ? 0 : 8, false, pixelColor);
            return;
        }
        const int x = 16;
        lcd->drawSymbol(SYMBOL_DEFEND, x, shieldTop ? 0 : 8, false, pixelColor);
        if (hit) lcd->drawSymbol(SYMBOL_ANGRY, x, lane ? 0 : 8, false, pixelColor);
        if (finisher) lcd->drawSymbol(SYMBOL_ANGRY, x, lane ? 8 : 0, false, pixelColor);
    }
}
