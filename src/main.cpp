/////////////////////////////////////////////////////////////////
/*
  main.cpp - main programm to test the VPetLCD Class and the
  screen classes. JUST FOR TESTING PURPOSES This is the *.ino file
  from the arduino ide
  Created by Berat Özdemir, January 16 , 2021.
*/
/////////////////////////////////////////////////////////////////

#include <cmath>
#include <Arduino.h>
#include "VPetLCD/VPetLCD.h"
#include "VPetLCD/VPetLCDMenuBar32p.h"
#include "VPetLCD/Screens/AgeWeightScreen.h"
#include "VPetLCD/Screens/DigimonNameScreen.h"
#include "VPetLCD/Screens/HeartsScreen.h"
#include "VPetLCD/Screens/ProgressBarScreen.h"
#include "VPetLCD/Screens/PercentageScreen.h"
#include "VPetLCD/Screens/SelectionScreen.h"
#include "VPetLCD/Screens/ClockScreen.h"
#include "VPetLCD/Screens/DigimonWatchingScreen.h"
#include "VPetLCD/Screens/AnimationScreens/EatingAnimationScreen.h"
#include "VPetLCD/Screens/AnimationScreens/SleepingAnimationScreen.h"
#include "VPetLCD/Screens/TrainingScreen.h"
#include "VPetLCD/Screens/AnimationScreens/CureAnimationScreen.h"
#include "VPetLCD/Screens/AnimationScreens/TrainingAnimationScreen.h"

#include "GameLogic/ScreenStateMachine.h"

#include "GameLogic/Digimon.h"
#include "GameLogic/EvolutionHandler.h"
#include "SaveGame/SaveGameHandler.h"
#include "SoundManager/SoundManager.h"


uint16_t digiIndex =DIGIMON_EGG;
Digimon digimon(digiIndex);
SaveGameHandler savegame;

//ESP32 Specific stuff
#include "VPetLCD/DisplayAdapter/TFT_eSPI_Displayadapter.h"
#include "VPetLCD/ESP32SpriteManager.h"
#include "GameLogic/ESP32DigimonDataLoader.h"

#include <TFT_eSPI.h>
#include "Button2.h"
#include <Arduino.h>


#define ADC_EN 14 //ADC_EN is the ADC detection enable port
#define ADC_PIN 34
#define BUTTON_1 35
#define BUTTON_2 0

// External passive piezo buzzer. Override with -D BUZZER_PIN=<gpio>.
#ifndef BUZZER_PIN
#define BUZZER_PIN 25
#endif

SoundManager soundManager(BUZZER_PIN);

Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

int hours = 12;
int minutes = 50;
int seconds = 0;

boolean buttonPressed = false;

int displayHeight = 240;
int displayWidth = 135;


//TFT_eSPI-Only stuff
TFT_eSPI tft = TFT_eSPI(displayWidth, displayHeight); // Create object "tft"
TFT_eSprite img = TFT_eSprite(&tft);                  // Create Sprite object "img" with pointer to "tft" object
TFT_eSPI_DisplayAdapter displayAdapter(&img, displayHeight, displayWidth);         //create a DisplayAdapter for VPetLCD class
//--------

//ESP32 Only stuff
ESP32SpriteManager spriteManager;
ESP32DigimonDataLoader dataLoader;
EvolutionHandler evolutionHandler;
//

//Creating all instances for the UI
VPetLCD screen(&displayAdapter, &spriteManager, 40, 16);
VPetLCDMenuBar32p menuBar(7,5,displayHeight);

V20::DigimonWatchingScreen digimonScreen(&evolutionHandler, &spriteManager, &digimon, -8, 40, 0, 0);
V20::DigimonNameScreen digiNameScreen(&spriteManager, dataLoader.getDigimonProperties(digiIndex)->digiName, digimon.getDigimonIndex(), 24);
V20::AgeWeightScreen ageWeightScreen(5, 21);
V20::HeartsScreen hungryScreen("Hungry", digimon.getHungerHearts(), 4);
V20::HeartsScreen strengthScreen("Str", digimon.getStrengthHearts(), 4);
V20::ProgressBarScreen energyScreen("Energy", 30, digimon.getEnergy());
V20::PercentageScreen sPercentageScreen("WIN", 'S', 100);
V20::PercentageScreen tPercentageScreen("WIN", 'T', 93);
V20::SelectionScreen foodSelection(true);
V20::SelectionScreen fightSelection(true);
V20::SelectionScreen lightSelection(true);
V20::SelectionScreen foodRefusal(false);
V20::SleepingAnimationScreen sleepingAnimationScreen(&spriteManager, digimon.getDigimonIndex());
V20::ClockScreen clockScreen(true);
CureAnimationScreen cureAnimationScreen(&spriteManager, &digimon);
V20::EatingAnimationScreen eatingAnimationScreen(&spriteManager, digimon.getDigimonIndex());
V20::TrainingScreen trainingSelection;
TrainingAnimationScreen trainingAnimationDefend(&spriteManager, digimon.getDigimonIndex(), &digimon);
TrainingAnimationScreen trainingAnimationAttack(&spriteManager, digimon.getDigimonIndex(), &digimon, 1);

//19 screens and 3 signals (next, confirm and back)
uint8_t numberOfScreens = 19;
uint8_t numberOfSignals = 3;

uint8_t confirmSignal = 0;
uint8_t nextSignal = 1;
uint8_t backSignal = 2;

//Creating the ScreenStateMachine, which handles transitions between screens
//and the actions of the buttons; the buttons are just sending signals to the statemachine
ScreenStateMachine stateMachine(numberOfScreens, numberOfSignals);

uint8_t digimonScreenId = stateMachine.addScreen(&digimonScreen);
uint8_t digiNameScreenId = stateMachine.addScreen(&digiNameScreen);
uint8_t ageWeightScreenId = stateMachine.addScreen(&ageWeightScreen);
uint8_t hungryScreenId = stateMachine.addScreen(&hungryScreen);
uint8_t strengthScreenId = stateMachine.addScreen(&strengthScreen);
uint8_t energyScreenId = stateMachine.addScreen(&energyScreen);
uint8_t sPercentageScreenId = stateMachine.addScreen(&sPercentageScreen);
uint8_t tPercentageScreenId = stateMachine.addScreen(&tPercentageScreen);
uint8_t foodSelectionId = stateMachine.addScreen(&foodSelection);
uint8_t fightSelectionId = stateMachine.addScreen(&fightSelection);
uint8_t lightSelectionId = stateMachine.addScreen(&lightSelection);
uint8_t foodRefusalId = stateMachine.addScreen(&foodRefusal);
uint8_t clockScreenId = stateMachine.addScreen(&clockScreen);
uint8_t eatingAnimationScreenId = stateMachine.addScreen(&eatingAnimationScreen);
uint8_t sleepingAnimationScreenId = stateMachine.addScreen(&sleepingAnimationScreen);
uint8_t trainingSelectionId = stateMachine.addScreen(&trainingSelection);
uint8_t trainingAnimationDefendId = stateMachine.addScreen(&trainingAnimationDefend);
uint8_t trainingAnimationAttackId = stateMachine.addScreen(&trainingAnimationAttack);
uint8_t cureAnimationScreenId = stateMachine.addScreen(&cureAnimationScreen);

uint8_t poop=0;

void stateMachineInit() {
  const DigimonProperties *properties = dataLoader.getDigimonProperties(digimon.getDigimonIndex());
  digimon.setProperties(properties);


  //return to food selection screen after showing eating animation
  eatingAnimationScreen.setAnimationEndAction([]() {
    stateMachine.setCurrentScreen(foodSelectionId);
  });

  //return to digimon watching screen after sleeping animation
  sleepingAnimationScreen.setAnimationEndAction([](){
    stateMachine.setCurrentScreen(digimonScreenId);
  });

  // in order to be able to go back to the digimon watching screen
  // we will add a transition from every screen to the digimon watching screen
  //triggered by the backsignal (backbutton)
  for (int i = 1; i <= numberOfScreens + 1;i++) {
    stateMachine.addTransition(i, digimonScreenId, backSignal);
  }

  //The Scale Menu transitions
  stateMachine.addTransition(digiNameScreenId, ageWeightScreenId, nextSignal);
  stateMachine.addTransition(ageWeightScreenId, hungryScreenId, nextSignal);
  stateMachine.addTransition(hungryScreenId, strengthScreenId, nextSignal);
  stateMachine.addTransition(strengthScreenId, energyScreenId, nextSignal);
  stateMachine.addTransition(energyScreenId, sPercentageScreenId, nextSignal);
  stateMachine.addTransition(sPercentageScreenId, tPercentageScreenId, nextSignal);
  stateMachine.addTransition(tPercentageScreenId, digiNameScreenId, nextSignal);

  //Transitions between clock screen and digimon watching screen
  stateMachine.addTransition(digimonScreenId, clockScreenId, backSignal);
  stateMachine.addTransition(clockScreenId, digimonScreenId, backSignal);

  //Conditional transtitions from digimonScreen to the others (menuselection)
  //this must be set, because unset transitions wont trigger transitionActions
  stateMachine.addTransition(digimonScreenId, digimonScreenId, nextSignal);

  //if nextSignal is sent (nextbutton pressed), the menuselection will be
  //incremented and the selection will be set
  stateMachine.addTransitionAction(digimonScreenId, nextSignal, []() {
      menuBar.nextSelection();
  });

  //Here are the conditional transitions handled.
  stateMachine.addTransition(digimonScreenId, digimonScreenId, confirmSignal);
  stateMachine.addTransitionAction(digimonScreenId, confirmSignal, []() {
    const uint8_t selected = menuBar.getSelection();
    // Eggs can only access the scales/stats menu.
    if (digimon.getState() == STATE_EGG && selected != 0) return;

    if (digimon.getState() == STATE_ASLEEP || !digimon.isLightsOn()) {
      if (selected != 0 && selected != 1 && selected != 2 && selected != 3 && selected != 5 &&
          !(selected == 6 && digimon.getState() == STATE_SICK)) return;
      if (selected >= 1 && selected <= 3 && digimon.disturbSleep()) {
        savegame.saveDigimon(&digimon);
      }
    }

    switch (menuBar.getSelection()) {
    case 0: // stats screen
      digiNameScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
      digiNameScreen.setDigimonName(digimon.getProperties()->digiName);
      hungryScreen.setHearts(digimon.getHungerHearts());
      strengthScreen.setHearts(digimon.getStrengthHearts());

      energyScreen.setFillPercentage(digimon.getEnergyPercentage());
      ageWeightScreen.setAge(digimon.getAge());
      ageWeightScreen.setWeight(digimon.getWeight());
      stateMachine.setCurrentScreen(digiNameScreenId);
      break;
    case 1: // feed
      if (digimon.getState() == STATE_EGG){
        break; // don't allow sleeping if still an egg
      }
      foodSelection.setSelection(0);
      stateMachine.setCurrentScreen(foodSelectionId);
      break;
    case 2: //train (this is not set up yet as no training logic)
      if (digimon.getState() == STATE_EGG) {
        break;
      }
      trainingSelection.setSelection(0);
      stateMachine.setCurrentScreen(trainingSelectionId);
      break;
    case 3: //fight (this is not set up yet as no fight logic)
      if (digimon.getState() == STATE_EGG){
        break; // don't allow sleeping if still an egg
      }
      fightSelection.setSelection(0);
      stateMachine.setCurrentScreen(fightSelectionId);
      break;
    case 4: // clean poop   
      Serial.println("Clean Poop button pressed");
      digimonScreen.flushPoop();
      digimon.setNumberOfPoops(0);
      savegame.saveDigimon(&digimon);
      break;
    case 5: //sleep -> ask for lights ON/OFF
      if (digimon.getState() == STATE_EGG){
        break; // don't allow sleeping if still an egg
      }
      lightSelection.setSelection(0);
      stateMachine.setCurrentScreen(lightSelectionId);
      break;
    case 6: // Cure sickness, or refuse with the training loss animation.
      cureAnimationScreen.start();
      if (digimon.getState() != STATE_SICK) soundManager.playAlert();
      stateMachine.setCurrentScreen(cureAnimationScreenId);
      break;
    }
    });

  //adding functionality of buttons in food  type screen:
  stateMachine.addTransition(foodSelectionId, foodSelectionId, nextSignal);
  stateMachine.addTransitionAction(foodSelectionId, nextSignal, []() {
    foodSelection.nextSelection();
  });

  //adding functionality of buttons in food screen:
  stateMachine.addTransition(foodSelectionId, foodSelectionId, confirmSignal);
  stateMachine.addTransitionAction(foodSelectionId, confirmSignal, []() {
    uint8_t selection = foodSelection.getSelection();
    switch (selection) {
    case 0:
      if (!digimon.feedMeal()) {
        stateMachine.setCurrentScreen(foodRefusalId);
        soundManager.playAlert();
        break;
      }
      savegame.saveDigimon(&digimon);
      eatingAnimationScreen.setSprites(SYMBOL_MEAT, SYMBOL_HALF_MEAT,SYMBOL_EMPTY_MEAT);
      eatingAnimationScreen.startAnimation();
      stateMachine.setCurrentScreen(eatingAnimationScreenId);
      break;
    case 1:
      if (!digimon.feedProtein()) {
        stateMachine.setCurrentScreen(foodRefusalId);
        soundManager.playAlert();
        break;
      }
      savegame.saveDigimon(&digimon);
      eatingAnimationScreen.setSprites(SYMBOL_PILL, SYMBOL_HALF_PILL,SYMBOL_EMPTY);
      eatingAnimationScreen.startAnimation();
      stateMachine.setCurrentScreen(eatingAnimationScreenId);
      break;
    }
    });

  stateMachine.addTransition(foodRefusalId, foodSelectionId, confirmSignal);
  stateMachine.addTransition(foodRefusalId, foodSelectionId, nextSignal);

  // Adjust the clock directly: next adds an hour, confirm adds a minute.
  auto applyClockChange = []() {
    seconds = 0;
    clockScreen.setHours(hours);
    clockScreen.setMinutes(minutes);
    clockScreen.setSeconds(seconds);
    digimon.updateSleepSchedule(hours, minutes, true);
    savegame.saveDigimon(&digimon);
  };
  stateMachine.addTransition(clockScreenId, clockScreenId, nextSignal);
  stateMachine.addTransitionAction(clockScreenId, nextSignal, [applyClockChange]() {
    hours = (hours + 1) % 24;
    applyClockChange();
  });
  stateMachine.addTransition(clockScreenId, clockScreenId, confirmSignal);
  stateMachine.addTransitionAction(clockScreenId, confirmSignal, [applyClockChange]() {
    minutes = (minutes + 1) % 60;
    applyClockChange();
  });

  cureAnimationScreen.setEndCallback([](bool treated) {
    if (treated && digimon.cure()) {
      digimon.updateSleepSchedule(hours, minutes);
      savegame.saveDigimon(&digimon);
      soundManager.playHappy();
    }
    stateMachine.setCurrentScreen(digimonScreenId);
  });

  // Training selection transitions
  stateMachine.addTransition(trainingSelectionId, trainingSelectionId, nextSignal);
  stateMachine.addTransitionAction(trainingSelectionId, nextSignal, []() {
    trainingSelection.nextSelection();
  });

  stateMachine.addTransition(trainingSelectionId, trainingSelectionId, confirmSignal);
  stateMachine.addTransitionAction(trainingSelectionId, confirmSignal, []() {
    uint8_t selection = trainingSelection.getSelection();
    switch (selection) {
    case 0: // Attack
      trainingAnimationAttack.startGame();
      savegame.saveDigimon(&digimon);
      stateMachine.setCurrentScreen(trainingAnimationAttackId);
      break;
    case 1: // Defence
      trainingAnimationDefend.startGame();
      savegame.saveDigimon(&digimon);
      stateMachine.setCurrentScreen(trainingAnimationDefendId);
      break;
    }
  });

  // Training animation input handling: map next/confirm to top/bottom choices
  stateMachine.addTransition(trainingAnimationDefendId, trainingAnimationDefendId, nextSignal);
  stateMachine.addTransitionAction(trainingAnimationDefendId, nextSignal, []() {
    trainingAnimationDefend.chooseShieldTop();
  });
  stateMachine.addTransition(trainingAnimationDefendId, trainingAnimationDefendId, confirmSignal);
  stateMachine.addTransitionAction(trainingAnimationDefendId, confirmSignal, []() {
    trainingAnimationDefend.chooseShieldBottom();
  });

  stateMachine.addTransition(trainingAnimationAttackId, trainingAnimationAttackId, nextSignal);
  stateMachine.addTransitionAction(trainingAnimationAttackId, nextSignal, []() {
    trainingAnimationAttack.chooseShieldTop();
  });
  stateMachine.addTransition(trainingAnimationAttackId, trainingAnimationAttackId, confirmSignal);
  stateMachine.addTransitionAction(trainingAnimationAttackId, confirmSignal, []() {
    trainingAnimationAttack.chooseShieldBottom();
  });

  // Play the result once, alongside the happy/angry animation in either mode.
  auto playTrainingResult = [](bool won) {
    savegame.saveDigimon(&digimon);
    Serial.println(won ? "Training result: WIN, requesting happy sound"
                       : "Training result: LOSS, requesting alert sound");
    if (won) soundManager.playHappy();
    else soundManager.playAlert();
  };
  trainingAnimationDefend.setResultCallback(playTrainingResult);
  trainingAnimationAttack.setResultCallback(playTrainingResult);

  // return to trainingSelection automatically when animation ends
  trainingAnimationDefend.setEndCallback([](){
    stateMachine.setCurrentScreen(trainingSelectionId);
  });
  trainingAnimationAttack.setEndCallback([](){
    stateMachine.setCurrentScreen(trainingSelectionId);
  });

  // Light on/off selection transitions
  stateMachine.addTransition(lightSelectionId, lightSelectionId, nextSignal);
  stateMachine.addTransitionAction(lightSelectionId, nextSignal, []() {
    lightSelection.nextSelection();
  });

  stateMachine.addTransition(lightSelectionId, lightSelectionId, confirmSignal);
  stateMachine.addTransitionAction(lightSelectionId, confirmSignal, []() {
    if (digimon.getState() == STATE_EGG) return;

    const bool lightsOn = lightSelection.getSelection() == 0;
    digimon.applyLights(lightsOn);
    stateMachine.setCurrentScreen(digimonScreenId);
    savegame.saveDigimon(&digimon);
  });

    //go back to food selection if pressed confirm again
    stateMachine.addTransition(eatingAnimationScreenId, foodSelectionId, confirmSignal);
    //abort animation
    stateMachine.addTransitionAction(eatingAnimationScreenId, confirmSignal, [](){
      eatingAnimationScreen.abortAnimation();
    });

  //adding functionality of buttons in fight screen:
  stateMachine.addTransition(fightSelectionId, fightSelectionId, nextSignal);
  stateMachine.addTransitionAction(fightSelectionId, nextSignal, []() {
    fightSelection.nextSelection();
  });

  // allow menu navigation while sleeping animation is active
  stateMachine.addTransition(sleepingAnimationScreenId, sleepingAnimationScreenId, nextSignal);
  stateMachine.addTransitionAction(sleepingAnimationScreenId, nextSignal, []() {
    menuBar.nextSelection();
  });

  // Use the same menu restrictions and actions from the sleeping screen.
  stateMachine.addTransition(sleepingAnimationScreenId, digimonScreenId, confirmSignal);
  stateMachine.addTransitionAction(sleepingAnimationScreenId, confirmSignal, []() {
    stateMachine.sendSignal(confirmSignal);
  });

}

void button_init()
{
  // Process every release directly: rapid taps must not become ignored
  // double/triple clicks, and a normal press must not be mistaken for Back.
  btn1.setReleasedHandler([](Button2& b) {
    if (b.wasPressedFor() >= 700) {
      stateMachine.sendSignal(backSignal);
    } else if (stateMachine.getCurrentScreenId() == clockScreenId) {
      stateMachine.sendSignal(nextSignal);
    }
    buttonPressed = true;
  });

  btn1.setPressedHandler([](Button2& b) {
    soundManager.playBeep();
    // Clock edits wait for release so holding Back cannot change the hour.
    if (stateMachine.getCurrentScreenId() != clockScreenId) stateMachine.sendSignal(nextSignal);
    buttonPressed = true;
  });

  btn2.setPressedHandler([](Button2& b) {
    soundManager.playBeep();
    stateMachine.sendSignal(confirmSignal);
    buttonPressed = true;
    });
}

void setupScreens()
{
  menuBar.setBarWidth(displayHeight - 16); // reserve room for the call bell
  menuBar.setIconOnIndex(0,0);
  menuBar.setIconOnIndex(1,1);
  menuBar.setIconOnIndex(2,2);
  menuBar.setIconOnIndex(3,3);
  menuBar.setIconOnIndex(4,4);
  menuBar.setIconOnIndex(5,5);
  menuBar.setIconOnIndex(6,6);


  screen.setMenuBar(&menuBar);
  screen.setLCDPos(0, 32);
  screen.setLcdScale(6);

  //Positioning of the screens
  int screensOffsetX = 4;


  //set offset of the screens
  ageWeightScreen.setPos(screensOffsetX, 0);
  strengthScreen.setPos(screensOffsetX, 0);
  hungryScreen.setPos(screensOffsetX, 0);
  energyScreen.setPos(screensOffsetX, 0);
  sPercentageScreen.setPos(screensOffsetX, 0);
  tPercentageScreen.setPos(screensOffsetX, 0);
  clockScreen.setPos(screensOffsetX, 0);
  eatingAnimationScreen.setPos(screensOffsetX, 0);
  sleepingAnimationScreen.setPos(screensOffsetX, 0);
  trainingSelection.setPos(screensOffsetX, 0);
  trainingAnimationDefend.setPos(screensOffsetX, 0);
  trainingAnimationAttack.setPos(screensOffsetX, 0);
  
  //adding the food selection options
  foodSelection.addOption("Meat", SYMBOL_MEAT);
  foodSelection.addOption("PILL", SYMBOL_PILL);

  //light selection options
  lightSelection.setShowIcons(false);
  lightSelection.addOption("ON");
  lightSelection.addOption("OFF");
  foodRefusal.setShowIcons(false);
  foodRefusal.addOption("FULL");

  //adding the battle options
  fightSelection.setShowIcons(false);
  fightSelection.addOption("SINGLE");
  fightSelection.addOption("TAG");

  //clockScreen
  clockScreen.setHours(hours);
  clockScreen.setMinutes(minutes);
  clockScreen.setSeconds(seconds);
}

// =========================================================================
void setup(void)
{
  Serial.begin(115200);
  Serial.println("Start");
  // GPIO35 uses the board's external pull-up. Sample only at startup.
  pinMode(BUTTON_1, INPUT);
  const bool resetButtonInitiallyHeld = digitalRead(BUTTON_1) == LOW;
  delay(50);
  const bool eraseSaveRequested = resetButtonInitiallyHeld && digitalRead(BUTTON_1) == LOW;
  if (!soundManager.begin()) {
    Serial.println("Buzzer initialization failed");
  }

  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  randomSeed(analogRead(1));
  
  hungryScreen.setMaxHearts(4);
  Serial.println(digimon.getState());

  savegame.init();
  const esp_reset_reason_t resetReason = esp_reset_reason();
  Serial.printf("Boot reset reason: %d\n", static_cast<int>(resetReason));
  if (eraseSaveRequested || resetReason == ESP_RST_EXT) {
    Serial.println("New game requested: clearing save and starting a new egg");
    savegame.resetDigimon(&digimon);
  } else {
    // Power-on restores the save; other resets also preserve progress.
    if (!savegame.loadDigimon(&digimon)) {
      Serial.println("No valid save: starting a new egg");
      savegame.resetDigimon(&digimon);
    } else {
      Serial.println(resetReason == ESP_RST_POWERON ? "Power-on: save restored" : "Save restored after reset");
    }
  }
  if (eraseSaveRequested) {
    Serial.println("Release the first game button to begin");
    // Do not feed the startup hold into the normal clock/Back handlers.
    do {
      while (digitalRead(BUTTON_1) == LOW) delay(10);
      delay(50);
    } while (digitalRead(BUTTON_1) == LOW);
  }
  // Screens were constructed before setup, when the pet was still an egg.
  eatingAnimationScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
  sleepingAnimationScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
  trainingAnimationDefend.setDigimonSpriteIndex(digimon.getDigimonIndex());
  trainingAnimationAttack.setDigimonSpriteIndex(digimon.getDigimonIndex());
  digiNameScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
  digiNameScreen.setDigimonName(digimon.getProperties()->digiName);

  //Some tft initialization stuff
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x86CE);

//init functions
Serial.println("button_init");
  button_init();
  Serial.println("setupScreens");
  setupScreens();
  


  stateMachineInit();
  Serial.println("stateMachineInit");

}
// =========================================================================

unsigned long ticker = 0;
unsigned long tickerResetValue = 1000;
unsigned long lastDelta = 0;
unsigned long clockAccMs = 0;
float getFragmentation() ;
boolean debug=false;

void loop()
{
  soundManager.update();
  //tft.fillScreen(0x86CE);
  unsigned long t1 = millis();

  const uint8_t previousHealthState = digimon.getState();
  const uint16_t previousCareMistakes = digimon.getCareMistakes();
  digimon.loop(lastDelta);
  if (digimon.getCareMistakes() != previousCareMistakes ||
      (previousHealthState != STATE_SICK && digimon.getState() == STATE_SICK)) {
    savegame.saveDigimon(&digimon);
  }


  //updating the screens which need the loop
  digimonScreen.loop(lastDelta);
  // The game clock below is the single source of time for ClockScreen.
  digiNameScreen.loop(lastDelta);

  //switch to next frame only when the screen is active
  if (stateMachine.getCurrentScreen() == &eatingAnimationScreen)
    eatingAnimationScreen.loop(lastDelta);
  if (stateMachine.getCurrentScreen() == &trainingAnimationDefend)
    trainingAnimationDefend.loop(lastDelta);
  if (stateMachine.getCurrentScreen() == &trainingAnimationAttack)
    trainingAnimationAttack.loop(lastDelta);
  
  if (stateMachine.getCurrentScreen() == &cureAnimationScreen)
    cureAnimationScreen.loop(lastDelta);

  // Menus remain readable without changing the pet's lights or sleep state.
  const uint8_t currentScreenId = stateMachine.getCurrentScreenId();
  screen.setForceBlackScreen(!digimon.isLightsOn() &&
      (currentScreenId == digimonScreenId || currentScreenId == sleepingAnimationScreenId));
  hungryScreen.setHearts(digimon.getHungerHearts());
  strengthScreen.setHearts(digimon.getStrengthHearts());
  ageWeightScreen.setAge(digimon.getAge());
  ageWeightScreen.setWeight(digimon.getWeight());
  energyScreen.setFillPercentage(digimon.getEnergyPercentage());
  screen.setCallActive(digimon.isCallActive());
  screen.renderScreen(stateMachine.getCurrentScreen());
  
  if (digimon.isEvolved()){
    digimon.setEvolved(false);
    digimonScreen.evolveDigimon();
    digimon.setProperties(dataLoader.getDigimonProperties(digimon.getDigimonIndex()));

    // Keep the chosen lights state and apply the evolved species' schedule.
    if (digimon.getState() == STATE_EGG) digimon.setState(STATE_AWAKE);
    digimon.updateSleepSchedule(hours, minutes);

    // Ensure animation screens and name screen use the new digimon index after evolution
    // (they were constructed with the old index and need to be updated)
    eatingAnimationScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
    sleepingAnimationScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
    trainingAnimationDefend.setDigimonSpriteIndex(digimon.getDigimonIndex());
    trainingAnimationAttack.setDigimonSpriteIndex(digimon.getDigimonIndex());
    digiNameScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
    savegame.saveDigimon(&digimon);
  }
  else{   
      // update internal clock and handle sleep/wake transitions once per second
      clockAccMs += lastDelta;
      while(clockAccMs >= 1000){
        clockAccMs -= 1000;
        seconds++;
        if(seconds >= 60){ seconds = 0; minutes++; }
        if(minutes >= 60){ minutes = 0; hours = (hours + 1) % 24; }
        clockScreen.setHours(hours);
        clockScreen.setMinutes(minutes);
        clockScreen.setSeconds(seconds);

        const uint8_t previousState = digimon.getState();
        digimon.updateSleepSchedule(hours, minutes);
        if (previousState != digimon.getState() &&
            (previousState == STATE_ASLEEP || digimon.getState() == STATE_ASLEEP)) {
          savegame.saveDigimon(&digimon);
          if (stateMachine.getCurrentScreenId() != clockScreenId) {
            stateMachine.setCurrentScreen(digimonScreenId);
          }
        }
    }
  }

  buttonPressed = false;

  
  if (debug == true)
  {
    //here should be debug stuff but its only fps lol
    tft.setTextColor(TFT_BLACK);
    tft.fillRect(0, 0, 100, 20, 0xFFFF);
    tft.drawString(String((1000.0) / lastDelta) + " FPS", 0, 0);
    tft.drawString( "Fragmentation: "+String(getFragmentation()) , 0, 10);
  
  }

  btn1.loop();
  btn2.loop();

  // Alert once per tired episode, without interrupting another sound.
  static bool tiredAlertPlayed = false;
  const bool needsLightsOff = digimon.getState() == STATE_TIRED && digimon.isLightsOn();
  if (!needsLightsOff) {
    tiredAlertPlayed = false;
  } else if (!tiredAlertPlayed && !soundManager.isPlaying()) {
    soundManager.playAlert();
    tiredAlertPlayed = true;
  }

  // A shared hunger/strength/sleep call is audible once during its care window.
  if (digimon.hasCallAlert() && !soundManager.isPlaying()) {
    soundManager.playAlert();
    digimon.acknowledgeCallAlert();
  }

  unsigned long t2 = millis();
  lastDelta = t2 - t1;
}

//measure the HeapFragmentation
float getFragmentation() {
  return 100 - heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) * 100.0 / heap_caps_get_free_size(MALLOC_CAP_8BIT);
}
