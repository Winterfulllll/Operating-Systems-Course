#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#pragma once

#include <algorithm>
#include <cstdlib>
#include <map>
#include <queue>
#include <random>
#include <string>
#include <vector>

struct GuessResult {
  int bulls;
  int cows;
};

GuessResult check_guess(const std::string &secret, const std::string &guess) {
  GuessResult result{0, 0};
  for (size_t i = 0; i < secret.size(); ++i) {
    if (secret[i] == guess[i]) {
      result.bulls++;
    } else if (std::find(secret.begin(), secret.end(), guess[i]) !=
               secret.end()) {
      result.cows++;
    }
  }
  return result;
}

std::string generate_secret(int length = 4) {
  std::string digits = "0123456789";
  std::shuffle(digits.begin(), digits.end(),
               std::default_random_engine(std::random_device{}()));
  return digits.substr(0, length);
}

#endif // GAME_LOGIC_H
