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
         lcd->draw16BitArray(digiSprite, screenX, screenY, true, pixelColor);
        });
    };
 }