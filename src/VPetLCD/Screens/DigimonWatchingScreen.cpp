/////////////////////////////////////////////////////////////////
/*
  Created by Berat Özdemir, January 24 , 2021.
*/
/////////////////////////////////////////////////////////////////

#include "DigimonWatchingScreen.h"
#include <Arduino.h>

V20::DigimonWatchingScreen::DigimonWatchingScreen(AbstractSpriteManager* _spriteManager, Digimon* _digimon, int8_t _minX, int8_t _maxX, int8_t _minY, int8_t _maxY) {
  setXLimitations(_minX, _maxX); // -8 32
  setYLimitations(_minY, _maxY);
  digimonX = 8;
  digimonY = 0;
  looksLeft = false;
  looksUp = false;
  probabilityChangeDirection = 20;
  probabilityChangeWalkingSprite = 60;
  probabilityMoveVertical = 25;
  probabilityMakeAnotherMove = 5;
  spriteManager = _spriteManager;
  poopAnimationCounter = 0;
  updateIntervallTime = 500;
  digimon = _digimon;
}

boolean V20::DigimonWatchingScreen::randomDecision(int percent) {
  return percent > random(0, 100);
}

void V20::DigimonWatchingScreen::evolveDigimon(){
  if(digimon->getDigimonIndex() < DIGIMON_AGUMON){
    digimon->setDigimonIndex(digimon->getDigimonIndex()+1);
  }
}

void V20::DigimonWatchingScreen::loop(long delta) {

  if(isNextFrameTime(delta)){
    if(isFlushing){
      if(poopOffsetY<16+8){
        poopOffsetY++;
      }else{
        isFlushing = false;
        poopOffsetY=0;
        numberOfPoopWhileFlushing=0;
        setUpdateIntervallTime(updateIntervallTime*10);
      }
    }else{
    poopAnimationCounter++;
    poopAnimationCounter%=2;
    calculateWalking();
    }
    // update sleeping symbol breathing offset each frame
    if(digimon->getState() == STATE_ASLEEP){
      if(sleepingSymbolGoingUp) sleepingSymbolOffset++;
      else sleepingSymbolOffset--;
      if(sleepingSymbolOffset > 1) sleepingSymbolGoingUp = false;
      if(sleepingSymbolOffset < -1) sleepingSymbolGoingUp = true;
    } else {
      sleepingSymbolOffset = 0;
      sleepingSymbolGoingUp = true;
    }
  }
}

void V20::DigimonWatchingScreen::calculateWalking() {
  bool isTired = (digimon->getState() == STATE_TIRED);
  bool isAsleep = (digimon->getState() == STATE_ASLEEP);
  bool isEgg = (digimon->getState() == STATE_EGG);

  // change facing only when not asleep
  if (!isEgg && !isAsleep && (randomDecision(probabilityChangeDirection) || digimonX < minX || digimonX > maxX - digimon->getNumberOfPoops() * poopWidth)) {
    looksLeft = !looksLeft;
  }

  if (isAsleep && (randomDecision(probabilityChangeDirection) || digimonY < minY || digimonY > maxY)) {
    looksUp = !looksUp;
  }

  if(isAsleep){
    // while asleep, use the sleeping sprite (handled in draw)
    currentWalkSprite = SPRITE_DIGIMON_SLEEPING;
  } else if(isTired){
    // while tired, use the tired sprite for display
    currentWalkSprite = SPRITE_DIGIMON_TIRED;
  } else {
    if (!isEgg && randomDecision(probabilityChangeWalkingSprite) || currentWalkSprite == 2) {
      //first two sprites are the walking animations
      currentWalkSprite++;
      currentWalkSprite %= 2;
    }
  }

  if (!isEgg && !isAsleep && randomDecision(probabilityMoveVertical)) {
    if (looksUp) {
      if (digimonY < maxY - 1) {
        digimonY++;
      }
      else {
        looksUp = !looksUp;
        if (digimonY > minY + 1)
          digimonY--;
      }
    }
    else {
      if (digimonY > minY + 1) {
        digimonY--;
      }
      else {
        looksUp = !looksUp;
        if (digimonY < maxY - 1)
          digimonY++;
      }
    }
  }

  if(!isEgg && !isAsleep){
    if (looksLeft) {
      if (digimonX > minX + 1) {
        digimonX--;
      }
      else {
        looksLeft = !looksLeft;
        if (digimonX < maxX - 1 - digimon->getNumberOfPoops() * poopWidth)
          digimonX++;
      }
    }
    else {
      if (digimonX < maxX - 1 - digimon->getNumberOfPoops() * poopWidth) {
        digimonX++;
      }
      else {
        looksLeft = !looksLeft;
        if (digimonX > minX + 1)
          digimonX--;
      }
    }
  }

  //with probability of 5% make some other moves (skip when tired/asleep)
  if (!isTired && !isAsleep && !isEgg && randomDecision(probabilityMakeAnotherMove)) {
    currentWalkSprite = SPRITE_DIGIMON_HAPPY;
  }

  // Enforce boundaries to avoid walking off the screen
  int maxAllowedX = maxX - 1 - digimon->getNumberOfPoops() * poopWidth;
  if(digimonX < minX) digimonX = minX;
  if(digimonX > maxAllowedX) digimonX = maxAllowedX;
  if(digimonY < minY) digimonY = minY;
  if(digimonY > maxY) digimonY = maxY;
}

/**
 * draws the poop
 * */
void V20::DigimonWatchingScreen::drawPoop(VPetLCD* lcd) {
  const byte* sprite = spriteManager->getSymbol(SYMBOL_POOP);
  const byte* flushSprite = spriteManager->getSymbol(SYMBOL_POOPWAVE);
  boolean mirrored = poopAnimationCounter == 1;
  uint8_t numPoop = max(numberOfPoopWhileFlushing, digimon->getNumberOfPoops()); //numberOfPoop is 0 right before/after flush begun

  for (int i = 0; i < numPoop; i++) {

    if (i % 2 == 0) {
      if(isFlushing)
        lcd->drawByteArray(flushSprite, poopWidth, poopWidth, screenX + maxX - poopWidth - poopWidth * i / 2, screenY - poopWidth+poopOffsetY, mirrored, pixelColor);
      lcd->drawByteArray(sprite, poopWidth, poopWidth, screenX + maxX - poopWidth - poopWidth * i / 2, screenY + poopWidth+poopOffsetY, mirrored, pixelColor);
    }
    else {
      lcd->drawByteArray(sprite, poopWidth, poopWidth, screenX + maxX - poopWidth - poopWidth * (i - 1) / 2, screenY+poopOffsetY, mirrored, pixelColor);
    }
  }
}

void V20::DigimonWatchingScreen::flushPoop(){
  if(digimon->getNumberOfPoops() > 0){
    numberOfPoopWhileFlushing = digimon->getNumberOfPoops();
    isFlushing = true;
    poopOffsetY=0;
    setUpdateIntervallTime(updateIntervallTime/10); // be careful of updateIntervalltimes which has 10 not as a factor
  }
}

/**
 * Draws the digimon Walking
 * */
void V20::DigimonWatchingScreen::drawWakedUp(VPetLCD* lcd) {
  const unsigned short* sprite = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), currentWalkSprite);
  lcd->draw16BitArray(sprite, screenX + digimonX, screenY + digimonY, !looksLeft, pixelColor);
}

void V20::DigimonWatchingScreen::drawSleeping(VPetLCD* lcd, boolean inBed) {
  const unsigned short* sprite = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), SPRITE_DIGIMON_SLEEPING);
  lcd->draw16BitArray(sprite, screenX + digimonX, screenY + digimonY, !looksLeft, pixelColor);
}

/**
 * draws the screen
 * */
void V20::DigimonWatchingScreen::draw(VPetLCD* lcd) {
  drawPoop(lcd);
  // show different sprites depending on digimon state
  if(digimon->getState() == STATE_ASLEEP) {
    // while asleep, keep the digimon horizontally centered and draw sleeping symbol relative to that
    int centerX = (minX + maxX) / 2; // center within allowed movement bounds
    // draw the sleeping sprite at the computed center position
    const unsigned short* sprite = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), SPRITE_DIGIMON_SLEEPING);
    lcd->draw16BitArray(sprite, screenX + centerX, screenY + digimonY, !looksLeft, pixelColor);

    // draw sleeping symbol so its bottom-center aligns with the digimon's top-center
    const int symbolW = SPRITES_SYMBOL_RESOLUTION;
    const int symbolH = SPRITES_SYMBOL_RESOLUTION;
    const int digiW = SPRITES_DIGIMON_RESOLUTION;
    int symbolY = screenY + digimonY - symbolH + sleepingSymbolOffset; // place above the digimon (with breathing offset)
    int symbolX;
    // place symbol to the side of the digimon depending on facing
    if(looksLeft){
      // place to left of digimon
      symbolX = screenX + centerX - symbolW; 
    } else {
      // place to right of digimon
      symbolX = screenX + centerX + digiW;
    }
    // clamp to screen bounds
    if(symbolX < 0) symbolX = 0;
    if(symbolY < 0) symbolY = 0;
    lcd->drawSymbol(SYMBOL_SLEEPING, symbolX, symbolY, false, pixelColor);
  } else if(digimon->getState() == STATE_TIRED) {
    const unsigned short* sprite = spriteManager->getDigimonSprite(digimon->getDigimonIndex(), SPRITE_DIGIMON_TIRED);
    lcd->draw16BitArray(sprite, screenX + digimonX, screenY + digimonY, !looksLeft, pixelColor);
  } else {
    drawWakedUp(lcd);
  }
}
