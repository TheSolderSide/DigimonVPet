#include "TrainingAnimationScreen.h"
#include "../../AbstractSpriteManager.h"
#include "../../ImageData/SymbolData.h"
#include "../../ImageData/DigimonSprites.h"
#include <Arduino.h>

// timing constants for training animation (ms)
static const unsigned long TRAINING_ATTACK_DURATION = 700;
static const unsigned long TRAINING_RESULT_HOLD = 600;
static const unsigned long TRAINING_ATTACK_FRAME = 300; // how long digimon shows attack sprite
// total flashing time after result (three hold periods by default)
static const unsigned long TRAINING_FLASH_TOTAL = TRAINING_RESULT_HOLD * 3;
static const unsigned long TRAINING_FLASH_PERIOD = 300;

TrainingAnimationScreen::TrainingAnimationScreen(AbstractSpriteManager* _spriteManager, uint16_t _digimonSpriteIndex, Digimon* _digimon, uint8_t _mode){
    spriteManager = _spriteManager;
    digimonSpriteIndex = _digimonSpriteIndex;
    digimon = _digimon;
    rounds = 3;
    currentRound = 0;
    blockedCount = 0;
    playerChoicePos = -1;
    opponentChoicePos = -1;
    stage = 0;
    stageTimer = 0;
    mode = (Mode)_mode;
}

void TrainingAnimationScreen::startGame(){
    currentRound = 0;
    blockedCount = 0;
    playerChoicePos = -1;
    opponentChoicePos = -1;
    stage = 0; // waiting for player
}

void TrainingAnimationScreen::chooseShieldTop(){
    if(stage != 0) return;
    playerChoicePos = 1;
    // opponent choice is random
    opponentChoicePos = random(0,2);

    // determine result: for DEFEND mode player succeeds when opponent attack matches shield
    // for ATTACK mode player succeeds when opponent shield does NOT match attack
    bool success = false;
    if(mode == MODE_DEFEND){
        // opponentChoicePos represents attack position
        if(opponentChoicePos == playerChoicePos) success = true;
    } else {
        // MODE_ATTACK: opponentChoicePos represents their shield
        if(opponentChoicePos != playerChoicePos) success = true;
    }
    if(success) blockedCount++;
    stage = 1; // animate
    stageTimer = 0;
}

void TrainingAnimationScreen::chooseShieldBottom(){
    if(stage != 0) return;
    playerChoicePos = 0;
    opponentChoicePos = random(0,2);

    bool success = false;
    if(mode == MODE_DEFEND){
        if(opponentChoicePos == playerChoicePos) success = true;
    } else {
        if(opponentChoicePos != playerChoicePos) success = true;
    }
    if(success) blockedCount++;
    stage = 1;
    stageTimer = 0;
}

void TrainingAnimationScreen::loop(unsigned long delta){
    if(stage == 1){
        stageTimer += delta;
        if(stageTimer > (TRAINING_ATTACK_DURATION + TRAINING_RESULT_HOLD)){
            // end of this round
            currentRound++;
            if(currentRound >= rounds){
                // finished
                if(blockedCount > (rounds/2)){
                    // award
                    if(digimon){
                        digimon->setTrainingCounter(digimon->getTrainingCounter()+1);
                        digimon->addDigimonPower(10);
                    }
                    stage = 2; // success animation
                    stageTimer = 0;
                } else {
                    stage = 3; // finished no success
                    stageTimer = 0;
                }
                // do not call endCallback here; wait until end animation has been shown
            } else {
                // next round
                playerChoicePos = -1;
                opponentChoicePos = -1;
                stage = 0;
                stageTimer = 0;
            }
        }
    } else if(stage == 2 || stage == 3){
        stageTimer += delta;
        // after flashing success/fail show, go to idle (-1)
        if(stageTimer > 1800){
            stage = 255; // finished
            if(endCallback) endCallback();
        }
    }
}


void TrainingAnimationScreen::draw(VPetLCD* lcd){
    // draw digimon further left so only half visible
    const unsigned short* digiSprite = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), SPRITE_DIGIMON_WALK_0);
    uint16_t screenX = getPosX();
    uint16_t screenY = getPosY();
    uint16_t pxColor = getPixelColor();

    int16_t digiWidth = SPRITES_DIGIMON_RESOLUTION;
    int16_t digiHeight = SPRITES_DIGIMON_RESOLUTION;
    int16_t digiDrawX = screenX - (digiWidth / 2); // desired half-visible position
    // ensure the digimon isn't completely off-screen; nudge right if needed
    if(digiDrawX < 2) digiDrawX = 2;
    // align the bottom of the digimon sprite with the bottom of the virtual LCD
    const int virtualHeight = 16; // virtual LCD height used by this project
    int16_t digiDrawY = screenY + (virtualHeight - digiHeight);

    // choose which digimon sprite to draw depending on stage/timer
    int spriteToShow = SPRITE_DIGIMON_WALK_0;
    if(stage == 1 && stageTimer < TRAINING_ATTACK_FRAME){
        spriteToShow = SPRITE_DIGIMON_ATTACK_1;
    }

    // for end animations (success/fail) we'll flash between happy/angry and walk

    // default draw (will be overridden for flashing below)
    const unsigned short* spriteArr = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), spriteToShow);
    lcd->draw16BitArray(spriteArr, digiDrawX, digiDrawY, true, pxColor);

    // positions for shield/attacks
    int16_t shieldNearX = digiDrawX + digiWidth + 2; // very close to digimon on right
    int16_t opponentAreaX = screenX + 48; // right side area for opponent

    // hints when waiting for player
    if(playerChoicePos == -1){
        if(mode == MODE_DEFEND){
            lcd->drawCharArrayOnLCD((char*)"Top:Next", screenX + 18, screenY+1, pxColor);
            lcd->drawCharArrayOnLCD((char*)"Bot:Confirm", screenX + 18, screenY+SPRITES_DIGIMON_RESOLUTION+1, pxColor);
        } else {
            lcd->drawCharArrayOnLCD((char*)"Fire:Next", screenX + 18, screenY+1, pxColor);
            lcd->drawCharArrayOnLCD((char*)"Aim:Confirm", screenX + 18, screenY+SPRITES_DIGIMON_RESOLUTION+1, pxColor);
        }
    }

    // compute top/bottom halves for attacks: top half [screenY .. screenY+7], bottom half [screenY+8 .. screenY+15]
    int half = virtualHeight / 2;
    int16_t topY = screenY + 1;
    int16_t bottomY = screenY + half + 1;

    // draw player shield close to digimon if chosen (defend mode)
    if(playerChoicePos != -1 && mode == MODE_DEFEND){
        int16_t shieldY = (playerChoicePos == 1 ? topY : bottomY);
        lcd->drawSymbol(SYMBOL_DEFEND, shieldNearX, shieldY, false, pxColor);
    }

    // draw opponent shield if in attack mode
    if(stage == 1 && mode == MODE_ATTACK){
        int16_t opShieldY = (opponentChoicePos == 1 ? topY : bottomY);
        lcd->drawSymbol(SYMBOL_DEFEND, opponentAreaX, opShieldY, false, pxColor);
    }

    // animate attack moving across when stage==1
    if(stage == 1){
        float t = (float)min((unsigned long)stageTimer, TRAINING_ATTACK_DURATION) / (float)TRAINING_ATTACK_DURATION;

        if(mode == MODE_DEFEND){
            // opponent attack moves left from right to shield/digimon
            int16_t startX = screenX + 64;
            int16_t targetX = shieldNearX; // hits shield/digimon area
            int16_t curX = startX - (int16_t)((startX - targetX) * t);
            int16_t attackY = (opponentChoicePos == 1 ? topY : bottomY);
            lcd->drawSymbol(SYMBOL_ATTACK, curX, attackY, false, pxColor);
            // when close to target show result marker centered above digimon for the hold duration
            if(stageTimer >= TRAINING_ATTACK_DURATION && stageTimer < (TRAINING_ATTACK_DURATION + TRAINING_RESULT_HOLD)){
                int16_t resultX = digiDrawX + (digiWidth/2) - (SPRITES_SYMBOL_RESOLUTION/2);
                // put result symbol centered in the top half of the virtual LCD
                int16_t resultY = screenY + (half/2) - (SPRITES_SYMBOL_RESOLUTION/2);
                if(opponentChoicePos == playerChoicePos){
                    lcd->drawSymbol(SYMBOL_SUCCESS, resultX, resultY, false, pxColor);
                } else {
                    lcd->drawSymbol(SYMBOL_ANGRY, resultX, resultY, false, pxColor);
                }
            }
        } else { // MODE_ATTACK
            // player attack moves right from near digimon to opponent area
            int16_t startX = digiDrawX + digiWidth + 4;
            int16_t targetX = opponentAreaX;
            int16_t curX = startX + (int16_t)((targetX - startX) * t);
            int16_t attackY = (playerChoicePos == 1 ? topY : bottomY);
            // mirror the attack symbol so it faces the other direction when attacking
            lcd->drawSymbol(SYMBOL_ATTACK, curX, attackY, true, pxColor);
            if(stageTimer >= TRAINING_ATTACK_DURATION && stageTimer < (TRAINING_ATTACK_DURATION + TRAINING_RESULT_HOLD)){
                int16_t resultX = digiDrawX + (digiWidth/2) - (SPRITES_SYMBOL_RESOLUTION/2);
                int16_t resultY = screenY + (half/2) - (SPRITES_SYMBOL_RESOLUTION/2);
                if(opponentChoicePos != playerChoicePos){
                    lcd->drawSymbol(SYMBOL_SUCCESS, resultX, resultY, false, pxColor);
                } else {
                    lcd->drawSymbol(SYMBOL_ANGRY, resultX, resultY, false, pxColor);
                }
            }
        }
    }

    // after game end show success digimon happy animation shortly
    // after game end show success/fail as a flashing animation
    if(stage == 2 || stage == 3){
        // determine flash phase and ensure flashing only for the configured total
        bool showResultPhase = (stageTimer < TRAINING_FLASH_TOTAL) && (((stageTimer / TRAINING_FLASH_PERIOD) % 2) == 0);
                int16_t resultX = digiDrawX + (digiWidth/2) - (SPRITES_SYMBOL_RESOLUTION/2);
                int16_t resultY = screenY + (half/2) - (SPRITES_SYMBOL_RESOLUTION/2);

        if(showResultPhase){
            // show happy/angry sprite and symbol
            if(stage == 2){
                const unsigned short* happy = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), SPRITE_DIGIMON_HAPPY);
                lcd->draw16BitArray(happy, digiDrawX, digiDrawY, true, pxColor);
                lcd->drawSymbol(SYMBOL_SUCCESS, resultX, resultY, false, pxColor);
            } else {
                const unsigned short* angry = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), SPRITE_DIGIMON_ANGRY_1);
                lcd->draw16BitArray(angry, digiDrawX, digiDrawY, true, pxColor);
                lcd->drawSymbol(SYMBOL_ANGRY, resultX, resultY, false, pxColor);
            }
        } else {
            // show default walk frame (no symbol)
            const unsigned short* walk = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), SPRITE_DIGIMON_WALK_0);
            lcd->draw16BitArray(walk, digiDrawX, digiDrawY, true, pxColor);
        }
    }
}

