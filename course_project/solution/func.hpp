#ifndef FUNC
#define FUNC

#pragma once

#include "shared_memory.hpp"
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

std::string generate_separator(int n) {
  std::string line(n, '=');
  line += '\n';
  return line;
}

std::vector<std::string> split(const std::string &str, char separator = ' ') {
  std::vector<std::string> words;
  std::string word;
  std::istringstream iss(str);
  while (std::getline(iss, word, separator)) {
    words.push_back(word);
  }
  return words;
}

bool is_number(const std::string &s) {
  return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                         return !std::isdigit(c);
                       }) == s.end();
}

bool is_invalid_create(const std::vector<std::string> &request_args) {
  if (request_args.size() < 2) {
    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
               "Error: <room_name> argument is required for 'create'!\n");
    return true;
  }

  int players_count = 1;
  if (request_args.size() >= 3) {
    if (!is_number(request_args[2])) {
      fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                 "Error: <players_count> must be a number!\n");
      return true;
    }
    players_count = std::stoi(request_args[2]);
    if (players_count < 1 || players_count > 5) {
      fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                 "Error: <players_count> must be between 1 and 5!\n");
      return true;
    }
  }

  int digits_count = 4;
  if (request_args.size() >= 4) {
    if (!is_number(request_args[3])) {
      fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                 "Error: <digits_count> must be a number!\n");
      return true;
    }
    digits_count = std::stoi(request_args[3]);
    if (digits_count < 2 || digits_count > 10) {
      fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                 "Error: <digits_count> must be between 2 and 10!\n");
      return true;
    }
  }

  return false;
}

bool is_invalid_enter(const std::vector<std::string> &request_args) {
  if (request_args.size() < 2) {
    fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
               "Error: <room_name> argument is required for 'enter'.\n");
    return true;
  }
  return false;
}

bool is_invalid_num(const std::string &request, int secret_length) {
  if (request.length() != static_cast<std::string::size_type>(secret_length) ||
      !std::all_of(request.begin(), request.end(), ::isdigit)) {
    return true;
  }

  return false;
}

#endif // FUNC
