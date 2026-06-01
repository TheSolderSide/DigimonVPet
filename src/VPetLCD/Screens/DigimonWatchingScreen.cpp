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
  }
}

void V20::DigimonWatchingScreen::calculateWalking() {

  if (digimon->getState() != 0 && randomDecision(probabilityChangeDirection) || digimonX < minX || digimonX > maxX - digimon->getNumberOfPoops() * poopWidth) {
    looksLeft = !looksLeft;
  }

  if (digimon->getState() != 0 && randomDecision(probabilityChangeDirection) || digimonY < minY || digimonY > maxY) {
    looksUp = !looksUp;
  }

  if (randomDecision(probabilityChangeWalkingSprite) || currentWalkSprite == 2) {
    //first two sprites are the walking animations
    currentWalkSprite++;
    currentWalkSprite %= 2;
  }

  if (digimon->getState() != 0 && randomDecision(probabilityMoveVertical)) {
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

  if(digimon->getState() != 0){
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

  //with probability of 5% make some other moves
  if (randomDecision(probabilityMakeAnotherMove)) {
    currentWalkSprite = SPRITE_DIGIMON_HAPPY;
  }
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
  drawWakedUp(lcd);
}
