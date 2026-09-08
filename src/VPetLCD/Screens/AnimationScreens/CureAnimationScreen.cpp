#include "CureAnimationScreen.h"

CureAnimationScreen::CureAnimationScreen(AbstractSpriteManager* sprites, Digimon* pet)
    : spriteManager(sprites), digimon(pet), failureAnimation(sprites, pet->getDigimonIndex(), pet) {
    failureAnimation.setEndCallback([this]() {
        active = false;
        if (endCallback) endCallback(false);
    });
}

void CureAnimationScreen::start() {
    elapsed = 0;
    active = true;
    treating = digimon->getState() == STATE_SICK;
    if (!treating) failureAnimation.showResult(false);
}

void CureAnimationScreen::loop(unsigned long delta) {
    if (!active) return;
    if (!treating) { failureAnimation.loop(delta); return; }
    elapsed += delta;
    if (elapsed >= 2500) {
        active = false;
        if (endCallback) endCallback(true);
    }
}

void CureAnimationScreen::draw(VPetLCD* lcd) {
    if (!treating) { failureAnimation.draw(lcd); return; }
    const int petX = 4;
    const auto sprite = elapsed >= 2000 ? SPRITE_DIGIMON_HAPPY : SPRITE_DIGIMON_TIRED;
    lcd->draw16BitArray(spriteManager->getDigimonSprite(digimon->getDigimonIndex(), sprite),
                        petX, 0, true, pixelColor);
    if (elapsed >= 2000) {
        lcd->drawSymbol(SYMBOL_SUCCESS, petX + SPRITES_DIGIMON_RESOLUTION + 2, 0, false, pixelColor);
        return;
    }
    // Needle approaches from the right, touches the pet, then withdraws.
    int needleX = 20;
    if (elapsed < 600) needleX += 6 - elapsed / 100;
    else if (elapsed >= 1400) needleX += (elapsed - 1400) / 100;
    const int y = 5;
    const int plungerX = elapsed < 1000 ? 11 : 9;
    for (int x = 0; x < 4; ++x) lcd->drawPixelOnLCD(needleX + x, y + 3, pixelColor);
    for (int x = 4; x <= 8; ++x) {
        lcd->drawPixelOnLCD(needleX + x, y + 1, pixelColor);
        lcd->drawPixelOnLCD(needleX + x, y + 5, pixelColor);
    }
    for (int row = 1; row <= 5; ++row) {
        lcd->drawPixelOnLCD(needleX + 4, y + row, pixelColor);
        lcd->drawPixelOnLCD(needleX + 8, y + row, pixelColor);
        lcd->drawPixelOnLCD(needleX + plungerX, y + row, pixelColor);
    }
    for (int x = 8; x <= plungerX; ++x) lcd->drawPixelOnLCD(needleX + x, y + 3, pixelColor);
}
