#include "SleepingAnimationScreen.h"



namespace V20 {

    SleepingAnimationScreen::SleepingAnimationScreen(AbstractSpriteManager *_spriteManager, uint16_t _digimonSpriteIndex) : AnimationScreen(_spriteManager, _digimonSpriteIndex,5){        
        addFrame([](VPetLCD* lcd, AnimationScreen* context) {
        AbstractSpriteManager* sm = context->getSpriteManager();
        uint16_t pixelColor = context->getPixelColor();
        uint16_t digiIndex = context->getDigimonSpriteIndex();

        const unsigned short* digiSprite = sm->getDigimonSprite(digiIndex, SPRITE_DIGIMON_SLEEPING);
        uint16_t screenX = context->getPosX();
        uint16_t screenY = context->getPosY();
                // draw digimon (mirrored)
                bool mirror = true;
                lcd->draw16BitArray(digiSprite, screenX, screenY, mirror, pixelColor);
                // draw sleeping symbol offset to the side the digimon is facing
                const int symbolW = SPRITES_SYMBOL_RESOLUTION;
                const int symbolH = SPRITES_SYMBOL_RESOLUTION;
                const int digiW = SPRITES_DIGIMON_RESOLUTION;
                int symbolY = screenY - symbolH;
                int symbolX;
                if(mirror){
                    // mirrored -> place symbol to right
                    symbolX = screenX + digiW;
                } else {
                    // normal -> place to left
                    symbolX = screenX - symbolW;
                }
                if(symbolX < 0) symbolX = 0;
                if(symbolY < 0) symbolY = 0;
                lcd->drawSymbol(SYMBOL_SLEEPING, symbolX, symbolY, false, pixelColor);
        });
    };
 }