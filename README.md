# Digimon V-Pet for ESP32/Arduino
This is an early version of a Digimon VPet for the ESP32 and other Arduino compatible devices
At the moment there are just [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI/) compatible Displays supported. But you can make your own Display working by just implementing an realization of the abstract DisplayAdapter Class with just 8 Methods.
The Goal of this project is to implement a Digimon VPet with all the functionality of real VPets and compatibility to real Digimon VPets.

<img src="screenshot.jpg" width="500" >
At the moment there is just the UI implemented but no functionality. But the Long Term goals are:

Hardware independence:
- the ardware should be highly customizable, so you can use different displays, different inputs etc. 

Functionality:
- Internet connectivity through WiFi
- [A-/D-Com](https://www.alphahub.site/guide) functionality 
- it should have a function to act as a gate, to allow 2 people to fight/jogress/... with their original VPETs through the internet
- it should be compatible to real VPETs (so you can fight/jogress/... ESP-VPET vs Original-VPET)
- ability to connect via bluetooth to other ESP32 Vpets to fight/jogress/...

Customizable: 
- it should be easy to add new digimon/pets/evolution lines
- it should be easy to change game mechanics and the UI

# How to install
## Sounds

Connect an external low-current passive piezo buzzer to GPIO 25 and GND
(use a suitable driver circuit for a speaker or higher-current buzzer).
A passive buzzer is needed to reproduce the different pitches.
Change `BUZZER_PIN` in `src/main.cpp`, or add `-D BUZZER_PIN=25` to
PlatformIO `build_flags`, to match your wiring.

The sound manager is initialized in `setup()` and updated in `loop()`.
Both buttons already beep when pressed. Call these functions from game events:

```cpp
soundManager.playBeep();  // Sound 1: short button beep
soundManager.playAlert(); // Sound 2: alternating alert / angry tones
soundManager.playHappy(); // Sound 3: rising celebration melody
```

You can also use `soundManager.play(SoundManager::Sound::Alert)`.
Playback uses PWM without delays; keep calling `update()` regularly for note timing.
Alerts and celebrations replace the current sound; button beeps do not interrupt them.
Call `stop()` to silence playback, or `setEnabled(false)` to mute and
`setEnabled(true)` to unmute. Calls before `begin()` are safely ignored.
The default LEDC channel is 2; reserve it and its paired channel 3 for sound
on the original ESP32, since they share a PWM timer.

## on ESP32 (TTGO T-Display)
Clone the Repo into VSCode/PlatformIO and just flash it to your device. Don't forget to configure your TFT_eSPI library properly (uncommenting/commenting the right line in user_setup_select.h). If you want to use Arduino IDE: the content of main.cpp is equal to arduinos *.ino files. 

## on other Devices:
Clone the Repo and configure TFT_eSPI library properly (uncommenting/commenting the right line in user_setup_select.h). Then change the define macros for the buttons according to your wiring. 


## Care, feeding and evolution

This project implements **Version 1** rules only.

- Empty visible hunger or strength hearts light the small red call bell in the
  top bar and request an alert. After **20 minutes** unresolved, the bell goes out
  and one care mistake is recorded. Hunger, strength and sleeping with lights on
  share one continuous episode: overlapping needs and prolonged neglect cannot
  repeatedly increase the counter. Resolve every active need to rearm it.
- The Digimon gets tired and alerts 30 minutes before `sleepHour`. It falls
  asleep at `sleepHour`, even with lights on; leaving them on uses the same
  20-minute care window. OFF sleeps immediately. At `wakeUpHour` it wakes and
  turns the lights on automatically, without a disturbance.
- While asleep, stats and lights remain usable. Opening Food, Training or Battle
  wakes it, turns lights on and adds one disturbance. It stays awake until OFF
  or the following bedtime. Battle remains a menu placeholder.
- From home, hold the first button to open the clock. Short-click the first
  button to advance the hour, press the second to advance the minute, and hold
  the first to return. Setting daytime wakes it without a disturbance.
- Meals increase fullness until the species' `stomachCapacity`, then show FULL
  without changing weight or counters. Reaching that limit counts one overfeed.
  Another overfeed can only count after a visible hunger heart has been lost.
  The existing visible-heart scale stays 0?10; capacity is at least 10, with
  larger species able to eat beyond four visible hearts. Protein restores
  strength, is refused at full strength, and does not count as overfeeding.
- Training has five rounds. Version 1 counts at session start, including a loss
  or cancellation. Winning restores strength without counting the session again.
- Hunger and strength each lose one internal point per `feedTimeSec` while
  awake (currently **600 seconds**, configurable in `DIGIMON_DATA`). Sleep
  pauses depletion, without filling already-empty hearts. Poop accumulates up to
  eight piles; reaching eight without cleaning causes sickness, not an extra
  care mistake. Cleaning resets the pile count; use Cure to treat sickness.

The species initializers previously omitted `feedTimeSec`, shifting the fields
that followed it; those rows now explicitly contain 600. Mamemon's invalid
sleep hour of 45 has been set to 19, matching the other current species.

Save/load now includes overfeeds, disturbances, the overfeeding latch and the
shared care episode. A versioned extension also keeps intact timer values away
from the legacy EEPROM layout's overlapping addresses. Loading a legacy save
preserves its existing counters but restarts its unrecoverable evolution timer
and initializes the new counters. Startup restores saved data on ordinary power-on or Reset. To start a new
pet, hold the **first game button (GPIO35)** while pressing Reset, then release
it after boot. Holding that button while powering on also clears the save.
The startup hold is consumed before normal input handling, so it cannot open
the clock or exit a menu. Normal gameplay holds are unchanged.

Blank or invalid saves start a fresh egg. `ESP_RST_EXT` also requests a fresh
egg on chips that support it; the original ESP32 reports its Reset button as
`ESP_RST_POWERON`. The game clock still starts at its configured default;
offline progression is not implemented.

The evolution thresholds themselves are unchanged. In `EvolutionHandler.cpp`,
Betamon's Meramon condition catches every care<=4/training<=48 case before the
Airdramon and Seadramon checks, making those two branches unreachable. Agumon's
Devimon branch effectively only matches training=31 because Greymon is checked
first. Those thresholds need a separate decision before changing the roster.

### Native regression tests

These tests run the actual C++ care, feeding, schedule, training, save/load and
evolution logic with hardware stubs for Version 1:

```powershell
python -m pip install --target .pio/test-tools ziglang
python tests/native/run.py
```

Build the device firmware with `platformio run`. On hardware, check the call
bell and buzzer, FULL refusal, five-round training/cancel, sleeping menu actions,
and the automatic wake at the species' configured time.


### Energy

Effort stars have been removed. DP is now named Energy throughout the game.
The Energy bar displays the current amount as a percentage of the species'
`maxEnergy`. Existing saves retain their stored DP value as energy.

An uninterrupted, lights-off sleep covering the entire `sleepHour` to
`wakeUpHour` interval restores energy to the species maximum at wake-up.
Turn the lights off before bedtime to qualify. Short rests also recover one
energy per real minute asleep with the lights off, capped at maximum energy.
Changing the clock alone does not grant energy. Night qualification is
tracked while powered on; rebooting breaks the uninterrupted-night check.
Vitamins add 2 energy and successful training adds 10, capped at `maxEnergy`.
Vitamins can be taken at full strength if energy still needs replenishing.

### Single story tournament

Open Fight > SINGLE. The preview shows the next opponent and alternates with
their match number. Button 1 switches GO/REST; button 2 confirms. GO starts an
automatic five-round match, with both Digimon attacking once each round.
Each match costs five energy paid at the start. Sick, injured, sleeping and
dead pets cannot start a match. The outcome uses the pet's condition at entry.

Each attack rolls against a hit chance based on these tunable rules in
`src/GameLogic/BattleRules.h`:

- Power = 40 + 40 × energy percentage / 100 + 2 × training wins (capped at 20 wins),
  minus 6 for each missing hunger heart and each missing strength heart.
- Hit chance = 60 + (attacker power − defender power) / 2, clamped to 15–90%.
- Opponents start with full hearts, 65% energy and zero training wins; each
  tournament step adds 3 energy percentage points and two training wins.
- Most hits after all five rounds wins; equal hits produce a draw. A win/loss
  finishes with the winner's double missile, one in each half of the LCD.
  The finisher is cosmetic and does not add another round or hit.

The camera cuts between your pet on the left and the opponent on the right,
using the existing training attack, defence, missile and reaction assets.
There are 12 opponents, Agumon through Monzaemon in roster order. A win advances;
a draw or loss keeps the same opponent for a retry. Completing all opponents
adds one to the CHAMPS stat and shows a three-second flashing champion title
and happy Digimon animation after the finishing missiles. The tournament then
returns to GO/REST at opponent 1; the next fight starts only when you choose GO.
Match records and tournament wins accumulate for the current pet until a new game.
The title is saved before the celebration; restarting during it resumes the
celebration when SINGLE is opened without counting the win again. An older save
with a completed tournament is credited with one title.
Each completed loss has a 25% chance of causing sickness, saved with the result.
Use the cure menu before battling again. Wins and draws do not cause battle sickness.

On the result screen, button 2 returns to GO/REST. REST or holding button 1
returns home between matches. Turn lights OFF to recover energy, then return
to SINGLE to resume. Back is ignored during a match. A power interruption
keeps the paid energy cost; an unfinished match awards no record or progress.

Stats now show single win percentage and separate WINS and DRAWS counts.
Losses are saved and included in the percentage denominator. Training wins
are tracked separately from attempts; old saves begin the new counters at zero.
Progress, results and recovered energy are saved automatically. TAG remains a placeholder.

Run `python tests/native/run.py` for battle outcomes, energy costs, rest,
persistence, old-save migration and existing care/training regression checks.
On hardware, check missile direction, upper/lower finishing missiles, result
readability, GO/REST controls and resuming the next opponent after a reboot.
