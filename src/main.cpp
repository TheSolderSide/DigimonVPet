/////////////////////////////////////////////////////////////////
/*
  main.cpp - main programm to test the VPetLCD Class and the
  screen classes. JUST FOR TESTING PURPOSES This is the *.ino file
  from the arduino ide
  Created by Berat Özdemir, January 16 , 2021.
*/
/////////////////////////////////////////////////////////////////

#include <cmath>
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
#include "VPetLCD/Screens/AnimationScreens/TrainingAnimationScreen.h"


#include "GameLogic/ScreenStateMachine.h"

#include "GameLogic/Digimon.h"
#include "SaveGame/SaveGameHandler.h"


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
//

//Creating all instances for the UI
VPetLCD screen(&displayAdapter, &spriteManager, 40, 16);
VPetLCDMenuBar32p menuBar(7,5,displayHeight);

V20::DigimonWatchingScreen digimonScreen(&spriteManager, &digimon, -8, 40, 0, 0);
V20::DigimonNameScreen digiNameScreen(&spriteManager, dataLoader.getDigimonProperties(digiIndex)->digiName, digimon.getDigimonIndex(), 24);
V20::AgeWeightScreen ageWeightScreen(5, 21);
V20::HeartsScreen hungryScreen("Hungry", digimon.getHungerHearts(), 4);
V20::HeartsScreen strengthScreen("Str", digimon.getStrengthHearts(), 4);
V20::HeartsScreen effortScreen("Effort", digimon.getEffortHearts(), 4);
V20::ProgressBarScreen dpScreen("DP", 30, digimon.getDigimonPower());
V20::PercentageScreen sPercentageScreen("WIN", 'S', 100);
V20::PercentageScreen tPercentageScreen("WIN", 'T', 93);
V20::SelectionScreen foodSelection(true);
V20::SelectionScreen fightSelection(true);
V20::SelectionScreen lightSelection(true);
V20::SleepingAnimationScreen sleepingAnimationScreen(&spriteManager, digimon.getDigimonIndex());
V20::ClockScreen clockScreen(false);
V20::EatingAnimationScreen eatingAnimationScreen(&spriteManager, digimon.getDigimonIndex());
V20::TrainingScreen trainingSelection;
TrainingAnimationScreen trainingAnimationDefend(&spriteManager, digimon.getDigimonIndex(), &digimon);
TrainingAnimationScreen trainingAnimationAttack(&spriteManager, digimon.getDigimonIndex(), &digimon, 1);

//17 screens and 3 signals (one for each button)
uint8_t numberOfScreens = 18;
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
uint8_t effortScreenId = stateMachine.addScreen(&effortScreen);
uint8_t dpScreenId = stateMachine.addScreen(&dpScreen);
uint8_t sPercentageScreenId = stateMachine.addScreen(&sPercentageScreen);
uint8_t tPercentageScreenId = stateMachine.addScreen(&tPercentageScreen);
uint8_t foodSelectionId = stateMachine.addScreen(&foodSelection);
uint8_t fightSelectionId = stateMachine.addScreen(&fightSelection);
uint8_t lightSelectionId = stateMachine.addScreen(&lightSelection);
uint8_t clockScreenId = stateMachine.addScreen(&clockScreen);
uint8_t eatingAnimationScreenId = stateMachine.addScreen(&eatingAnimationScreen);
uint8_t sleepingAnimationScreenId = stateMachine.addScreen(&sleepingAnimationScreen);
uint8_t trainingSelectionId = stateMachine.addScreen(&trainingSelection);
uint8_t trainingAnimationDefendId = stateMachine.addScreen(&trainingAnimationDefend);
uint8_t trainingAnimationAttackId = stateMachine.addScreen(&trainingAnimationAttack);

uint8_t poop=0;
// flag to indicate lightSelection was opened from sleeping screen
bool lightSelectionOpenedFromSleep = false;

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
    screen.setForceBlackScreen(false);
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
  stateMachine.addTransition(strengthScreenId, effortScreenId, nextSignal);
  stateMachine.addTransition(effortScreenId, dpScreenId, nextSignal);
  stateMachine.addTransition(dpScreenId, sPercentageScreenId, nextSignal);
  stateMachine.addTransition(sPercentageScreenId, tPercentageScreenId, nextSignal);
  stateMachine.addTransition(tPercentageScreenId, digiNameScreenId, nextSignal);
  // Do not allow `nextSignal` to interrupt sleep; waking should be handled by confirm or animation end

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
    //uint8_t maxdp = digimon.getProperties()->maxDigimonPower;
    // If digimon is asleep, only allow stats (0) and sleep (5)
    if(digimon.getState() == STATE_ASLEEP){
      uint8_t sel = menuBar.getSelection();
      if(sel != 0 && sel != 5) return;
    }

    switch (menuBar.getSelection()) {
    case 0: // stats screen
      digiNameScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
      digiNameScreen.setDigimonName(digimon.getProperties()->digiName);
      hungryScreen.setHearts(digimon.getHungerHearts());
      strengthScreen.setHearts(digimon.getStrengthHearts());
      effortScreen.setHearts(digimon.getEffortHearts());

      dpScreen.setFillPercentage((digimon.getDigimonPower()));
      // if(maxdp > 0){
      // }else{
      //   dpScreen.setFillPercentage(0);
      // }
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
      break;
    case 5: //sleep -> ask for lights ON/OFF
      if (digimon.getState() == STATE_EGG){
        break; // don't allow sleeping if still an egg
      }
      lightSelection.setSelection(0);
      stateMachine.setCurrentScreen(lightSelectionId);
      break;
    case 6: //cure
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
      digimon.addWeight(1);
      digimon.increaseHunger(1);
      eatingAnimationScreen.setSprites(SYMBOL_MEAT, SYMBOL_HALF_MEAT,SYMBOL_EMPTY_MEAT);
      eatingAnimationScreen.startAnimation();
      stateMachine.setCurrentScreen(eatingAnimationScreenId);
      break;
    case 1:
      digimon.addWeight(2);
      digimon.addStrength(2);
      digimon.addDigimonPower(2);
      eatingAnimationScreen.setSprites(SYMBOL_PILL, SYMBOL_HALF_PILL,SYMBOL_EMPTY);
      eatingAnimationScreen.startAnimation();
      stateMachine.setCurrentScreen(eatingAnimationScreenId);
      break;
    }
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
      stateMachine.setCurrentScreen(trainingAnimationAttackId);
      break;
    case 1: // Defence
      trainingAnimationDefend.startGame();
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
    uint8_t selection = lightSelection.getSelection();
    Serial.println(String("Light selection confirm pressed, sel=") + String(selection));

    const DigimonProperties* props = digimon.getProperties();
    int currentMins = hours * 60 + minutes;
    int sleepMins = props->sleepHour * 60;
    int wakeMins = props->wakeUpHour * 60;
    bool inSleepWindow = false;
    if (sleepMins <= wakeMins) {
      inSleepWindow = (currentMins >= sleepMins && currentMins < wakeMins);
    } else {
      // overnight window (e.g., 22:00 -> 08:00)
      inSleepWindow = (currentMins >= sleepMins || currentMins < wakeMins);
    }

    switch(selection){
      case 0: // ON
        digimon.setLightsOn(true);
        screen.setForceBlackScreen(false);
        // If we opened the light menu from sleep, keep the digimon asleep and
        // reveal the light menu so the user can see the confirmation state.
        if(lightSelectionOpenedFromSleep){
          Serial.println("Light ON selected from sleep — keep asleep, reveal menu");
          savegame.saveDigimon(&digimon);
          // remain on the lightSelection screen so the user sees the result
          lightSelectionOpenedFromSleep = false;
        } else {
          // normal flow: turn lights on and restore display; set TIRED if not asleep
          digimon.setForcedAsleep(false);
          if(digimon.getState() != STATE_ASLEEP){
            Serial.println("hitting tired state from light selection number 1");
            if(inSleepWindow){
              digimon.setState(STATE_TIRED);
            } else {
              digimon.setState(STATE_AWAKE);         
            }
          screen.setForceBlackScreen(false);
          savegame.saveDigimon(&digimon);
          stateMachine.setCurrentScreen(digimonScreenId);
         }
        }
        break;
      case 1: // OFF
        digimon.setLightsOn(false);
        digimon.setForcedAsleep(true);
        screen.setForceBlackScreen(true);
        // only transition to ASLEEP if current time is within the digimon's sleep window
        {
          if (inSleepWindow) {
            digimon.setState(STATE_ASLEEP);
            sleepingAnimationScreen.startAnimation();
            stateMachine.setCurrentScreen(sleepingAnimationScreenId);
          } else {
            // don't force asleep outside sleeping hours; mark as tired if awake
            if (digimon.getState() == STATE_AWAKE) {
              Serial.println("hitting tired state from light selection number 2");
              digimon.setState(STATE_AWAKE);
            }
            stateMachine.setCurrentScreen(digimonScreenId);
          }
        }
        savegame.saveDigimon(&digimon);
        break;
    }
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

  //sleep functionality
  stateMachine.addTransition(sleepingAnimationScreenId, sleepingAnimationScreenId, confirmSignal);
  stateMachine.addTransitionAction(sleepingAnimationScreenId, confirmSignal, []() {
    Serial.println("Sleep button pressed");
    // If the menu is currently set to the Lights icon, open the light selection
    // instead of immediately waking — this lets the user choose ON/OFF while asleep.
    if(menuBar.getSelection() == 5){
      Serial.println("Opening light selection from sleeping screen");
      lightSelection.setSelection(0);
      // mark that we opened the light selection from the sleeping flow
      lightSelectionOpenedFromSleep = true;
      // reveal the light selection menu immediately so the user can see ON/OFF
      screen.setForceBlackScreen(false);
      stateMachine.setCurrentScreen(lightSelectionId);
      return;
    }

    Serial.println("Sleep button pressed");
    if(digimon.getState() != STATE_ASLEEP){
      Serial.println("going to Sleep");
      digimon.setState(STATE_ASLEEP);
      sleepingAnimationScreen.startAnimation();
      stateMachine.setCurrentScreen(sleepingAnimationScreenId);
      screen.setForceBlackScreen(true);
    } else {
      Serial.println("waking up");
      digimon.setState(STATE_AWAKE);
      digimon.setForcedAsleep(false);
      screen.setForceBlackScreen(false);
      stateMachine.setCurrentScreen(digimonScreenId);
    }
  }); 

}

void button_init()
{
  btn1.setLongClickHandler([](Button2& b) {
    stateMachine.sendSignal(backSignal);
    buttonPressed = true;
    });

  btn1.setPressedHandler([](Button2& b) {
    stateMachine.sendSignal(nextSignal);
    buttonPressed = true;
    });

  btn2.setPressedHandler([](Button2& b) {
    Serial.println("Button2 pressed: confirm handler");
    // if display is forced black, either forward the confirm to the state machine
    // when sleeping animation is active (so the user can select lights), or wake directly
    if(screen.isForceBlackScreen()){
      uint8_t curId = stateMachine.getCurrentScreenId();
      // If we're on the sleeping animation or the light selection, forward the confirm
      // so the user can interact with the light menu even while the VPET area is black.
      if(curId == sleepingAnimationScreenId || curId == lightSelectionId){
        Serial.println("Screen is black — forwarding confirm to state machine (sleep/light)");
        bool sent = stateMachine.sendSignal(confirmSignal);
        Serial.println(String("confirmSignal sent: ") + String(sent));
        VPetLCD::Screen* cur = stateMachine.getCurrentScreen();
        Serial.println(String("CurrentScreenId: ") + String(stateMachine.getCurrentScreenId()));
        if(cur == &lightSelection) Serial.println("Current screen: lightSelection");
        else if(cur == &digimonScreen) Serial.println("Current screen: digimonScreen");
        else if(cur == &sleepingAnimationScreen) Serial.println("Current screen: sleepingAnimationScreen");
        else if(cur == &eatingAnimationScreen) Serial.println("Current screen: eatingAnimationScreen");
        buttonPressed = true;
        return;
      }

      Serial.println("Screen is black — waking directly");
      digimon.setState(STATE_AWAKE);
      digimon.setForcedAsleep(false);
      screen.setForceBlackScreen(false);
      stateMachine.setCurrentScreen(digimonScreenId);
      buttonPressed = true;
      return;
    }
    Serial.println("sending confirmSignal");
    bool sent = stateMachine.sendSignal(confirmSignal);
    Serial.println(String("confirmSignal sent: ") + String(sent));
    // debug: print which screen is now active
    VPetLCD::Screen* cur = stateMachine.getCurrentScreen();
    Serial.println(String("CurrentScreenId: ") + String(stateMachine.getCurrentScreenId()));
    if(cur == &lightSelection) Serial.println("Current screen: lightSelection");
    else if(cur == &digimonScreen) Serial.println("Current screen: digimonScreen");
    else if(cur == &sleepingAnimationScreen) Serial.println("Current screen: sleepingAnimationScreen");
    else if(cur == &eatingAnimationScreen) Serial.println("Current screen: eatingAnimationScreen");
    else if(cur == &trainingSelection) Serial.println("Current screen: trainingSelection");
    else if(cur == &trainingAnimationDefend) Serial.println("Current screen: trainingAnimationDefend");
    else if(cur == &trainingAnimationAttack) Serial.println("Current screen: trainingAnimationAttack");
    else Serial.println("Current screen: unknown");
    buttonPressed = true;
    });
}

void setupScreens()
{
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
  effortScreen.setPos(screensOffsetX, 0);
  strengthScreen.setPos(screensOffsetX, 0);
  hungryScreen.setPos(screensOffsetX, 0);
  dpScreen.setPos(screensOffsetX, 0);
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

  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  randomSeed(analogRead(1));
  
  hungryScreen.setMaxHearts(4);
  Serial.println(digimon.getState());

  /*
  digimon.setDigimonIndex(10);
    digimon.setState(20);
    digimon.setAge(30);
    digimon.setWeight(40);
    digimon.setFeedCounter(50);
    digimon.setCareMistakes(60);
    digimon.setTrainingCounter(70);
    digimon.setTimeUntilEvolution(80);
    digimon.setPoopTimer(90);
    digimon.setAgeTimer(100);
    digimon.setEvolutionTimer(110);
    Serial.println(digimon.getDigimonIndex());
  Serial.println(digimon.getState());
  Serial.println(digimon.getAge());
  Serial.println(digimon.getWeight());
  Serial.println(digimon.getFeedCounter());
  Serial.println(digimon.getCareMistakes());
  Serial.println(digimon.getTrainingCounter());
  Serial.println(digimon.getTimeUntilEvolution());
  Serial.println(digimon.getPoopTimer());
  Serial.println(digimon.getAgeTimer());
  Serial.println(digimon.getEvolutionTimer());
  savegame.saveDigimon(&digimon);
  */

 //savegame.loadDigimon(&digimon);

  //Some tft initialization stuff
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(0x86CE);

//init functions
Serial.println("button_init");
  button_init();
  Serial.println("setupScreens");
  setupScreens();
  
  // init savegame (EEPROM)
  savegame.init();

  stateMachineInit();
  Serial.println("stateMachineInit");

}
// =========================================================================

unsigned long ticker = 0;
unsigned long tickerResetValue = 1000;
unsigned long lastDelta = 0;
unsigned long clockAccMs = 0;
unsigned long lastEvolutionMs = 0;
float getFragmentation() ;
boolean debug=false;

void loop()
{
  //tft.fillScreen(0x86CE);
  unsigned long t1 = millis();

  digimon.loop(lastDelta);


  //updating the screens which need the loop
  digimonScreen.loop(lastDelta);
  clockScreen.loop(lastDelta);
  digiNameScreen.loop(lastDelta);

  //switch to next frame only when the screen is active
  if (stateMachine.getCurrentScreen() == &eatingAnimationScreen)
    eatingAnimationScreen.loop(lastDelta);
  if (stateMachine.getCurrentScreen() == &trainingAnimationDefend)
    trainingAnimationDefend.loop(lastDelta);
  if (stateMachine.getCurrentScreen() == &trainingAnimationAttack)
    trainingAnimationAttack.loop(lastDelta);
  
  screen.renderScreen(stateMachine.getCurrentScreen());
  
  if (digimon.isEvolved()){
    digimon.setEvolved(false);
    digimonScreen.evolveDigimon();
    digimon.setProperties(dataLoader.getDigimonProperties(digimon.getDigimonIndex()));

    // if we just hatched/evolved from egg, set initial state for new digimon
    // prefer TIRED if we are near/after its sleep hour so the player can decide lights
    const DigimonProperties* newProps = digimon.getProperties();
    // set initial state for newly evolved digimon (regardless of previous state)
    int currentMins = hours * 60 + minutes;
    int sleepMins = newProps->sleepHour * 60;
    int minutesUntilSleep = (sleepMins - currentMins + 24*60) % (24*60);
    int minutesSinceSleep = digimon.getState() == STATE_EGG ? 0 : (currentMins - sleepMins + 24*60) % (24*60);

    if(minutesUntilSleep <= 30 || ((minutesSinceSleep > 0) && digimon.getState() != STATE_EGG)){
      digimon.setState(STATE_TIRED);
    } else {
      digimon.setState(STATE_AWAKE);
    }
    // record evolution time to avoid immediate auto-sleeping
    lastEvolutionMs = millis();
    digimon.setForcedAsleep(false);

    // Ensure animation screens and name screen use the new digimon index after evolution
    // (they were constructed with the old index and need to be updated)
    eatingAnimationScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
    sleepingAnimationScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
    trainingAnimationDefend.setDigimonSpriteIndex(digimon.getDigimonIndex());
    trainingAnimationAttack.setDigimonSpriteIndex(digimon.getDigimonIndex());
    digiNameScreen.setDigimonSpriteIndex(digimon.getDigimonIndex());
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

        const DigimonProperties* props = digimon.getProperties();
        if(props != NULL){
        int currentMins = hours * 60 + minutes;
        int sleepMins = props->sleepHour * 60;
        int wakeMins = props->wakeUpHour * 60;
        int minutesUntilSleep = (sleepMins - currentMins + 24*60) % (24*60);
        int minutesSinceSleep = digimon.getState() == STATE_EGG ? 0 : (currentMins - sleepMins + 24*60) % (24*60);

        // skip sleep logic for egg state
        if(digimon.getState() != STATE_EGG)
        {
          // 30 minutes before sleep -> TIRED (normal case)
          if(minutesUntilSleep == 30 && digimon.getState() == STATE_AWAKE && !digimon.isForcedAsleep()){
            digimon.setState(STATE_TIRED);
          }

          // If current time is after sleep time but within 30 minutes, ensure the digimon is TIRED
          if(minutesSinceSleep > 0 && minutesSinceSleep < 30 && digimon.getState() == STATE_AWAKE && !digimon.isForcedAsleep()){
            digimon.setState(STATE_TIRED);
          }

          // If 30 or more minutes have passed since sleep time:
          // - if lights are OFF -> transition to ASLEEP
          // - if lights are ON -> remain TIRED but count a care mistake (owner kept lights on)
          if(minutesSinceSleep >= 30 && minutesUntilSleep <=0)
          {
            if(digimon.isLightsOn() && lastEvolutionMs != 0 && (millis() - lastEvolutionMs) < 60000){
              // count care mistake once when lights are kept on past sleep
              if(!digimon.isSleepCareMistakeLogged()){
                digimon.setCareMistakes(digimon.getCareMistakes()+1);
                digimon.setSleepCareMistakeLogged(true);
              }
              // stay TIRED (do not auto-sleep while lights are on)
              digimon.setState(STATE_TIRED);
            } 
            else 
            {
              // lights are off -> allow sleeping, but don't auto-sleep immediately after evolution
              if(digimon.getState() != STATE_ASLEEP){
                bool allowSleep = true;
                if(lastEvolutionMs != 0 && (millis() - lastEvolutionMs) < 60000){
                  allowSleep = false;
                }
                if(allowSleep){
                  digimon.setState(STATE_ASLEEP);
                  sleepingAnimationScreen.startAnimation();
                  stateMachine.setCurrentScreen(sleepingAnimationScreenId);
                  // blacken display for auto-sleep as well
                  screen.setForceBlackScreen(true);
                } else {
                  // keep tired until player toggles lights or the grace period expires
                  digimon.setState(STATE_TIRED);
                }
              }
            }
          }
        }

        // wake up at wakeUpHour
        if(currentMins == wakeMins && digimon.getState() == STATE_ASLEEP){
          digimon.setState(STATE_AWAKE);
          digimon.setForcedAsleep(false);
          // restore lights to ON when waking and clear night's logged flag
          digimon.setLightsOn(true);
          // restore display when waking
          screen.setForceBlackScreen(false);
          digimon.setSleepCareMistakeLogged(false);
          savegame.saveDigimon(&digimon);
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

  unsigned long t2 = millis();
  lastDelta = t2 - t1;
}

//measure the HeapFragmentation
float getFragmentation() {
  return 100 - heap_caps_get_largest_free_block(MALLOC_CAP_8BIT) * 100.0 / heap_caps_get_free_size(MALLOC_CAP_8BIT);
}