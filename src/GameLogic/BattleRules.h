#pragma once
#include <stdint.h>

namespace BattleRules {
constexpr uint8_t ROUNDS = 5;
constexpr uint8_t ENERGY_COST = 5;
constexpr uint8_t OPPONENTS = 12;
constexpr uint8_t LOSS_SICKNESS_PERCENT = 25;

// Normalized energy prevents high-capacity species from automatically winning.
inline int power(unsigned energyPercent, unsigned trainingWins, unsigned hungerHearts, unsigned strengthHearts) {
    return 40 + (energyPercent > 100 ? 100 : energyPercent) * 40 / 100
        + (trainingWins > 20 ? 20 : trainingWins) * 2
        - (4 - (int)hungerHearts) * 6 - (4 - (int)strengthHearts) * 6;
}
inline int hitChance(int attacker, int defender) {
    const int chance = 60 + (attacker - defender) / 2;
    return chance < 15 ? 15 : (chance > 90 ? 90 : chance);
}
inline int result(unsigned playerHits, unsigned enemyHits) {
    return playerHits > enemyHits ? 1 : (playerHits < enemyHits ? -1 : 0);
}
}
