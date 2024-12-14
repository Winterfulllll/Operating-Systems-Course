#ifndef SHARED_MEMORY_HPP
#define SHARED_MEMORY_HPP

#pragma once

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#define SHARED_MEMORY_BASE "/bulls_and_cows_memory"

struct Game {
  char room_name[128];
  int players_in_room;
  int max_players;
  bool is_active;
};

struct SharedMemory {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  char command[256];
  char response[256];
  Game games[100];
  int games_count;
};

std::vector<Game> get_all_games() {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  std::vector<Game> all_games;

  pthread_mutex_lock(&shared_memory->mutex);

  for (int i = 0; i < shared_memory->games_count; i++) {
    all_games.push_back(shared_memory->games[i]);
  }

  pthread_mutex_unlock(&shared_memory->mutex);

  return all_games;
}

Game *find_game_by_name(std::vector<Game> &games, const char *target_name) {
  auto it = std::find_if(games.begin(), games.end(), [&](const Game &game) {
    return std::strcmp(game.room_name, target_name) == 0;
  });

  if (it != games.end()) {
    return &(*it);
  } else {
    return nullptr;
  }
}

bool check_game_exists_in_shm(const char *target_name) {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  bool found = false;

  pthread_mutex_lock(&shared_memory->mutex);

  for (int i = 0; i < shared_memory->games_count; ++i) {
    if (std::strcmp(shared_memory->games[i].room_name, target_name) == 0) {
      found = true;
      break;
    }
  }

  pthread_mutex_unlock(&shared_memory->mutex);

  return found;
}

bool add_game(const Game &new_game) {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  pthread_mutex_lock(&shared_memory->mutex);

  if (shared_memory->games_count >= 100) {
    pthread_mutex_unlock(&shared_memory->mutex);
    return false;
  }

  shared_memory->games[shared_memory->games_count] = new_game;
  shared_memory->games_count++;

  pthread_mutex_unlock(&shared_memory->mutex);

  return true;
}

bool remove_game(const char *room_name) {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  pthread_mutex_lock(&shared_memory->mutex);

  int index = -1;

  for (int i = 0; i < shared_memory->games_count; i++) {
    if (strcmp(shared_memory->games[i].room_name, room_name) == 0) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    pthread_mutex_unlock(&shared_memory->mutex);
    return false;
  }

  for (int i = index; i < shared_memory->games_count - 1; i++) {
    shared_memory->games[i] = shared_memory->games[i + 1];
  }
  shared_memory->games_count--;

  pthread_mutex_unlock(&shared_memory->mutex);

  return true;
}

bool toggle_game_state(const char *room_name) {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    return false;
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    return false;
  }

  bool toggled = false;
  pthread_mutex_lock(&shared_memory->mutex);
  for (int i = 0; i < shared_memory->games_count; ++i) {
    if (std::strcmp(shared_memory->games[i].room_name, room_name) == 0) {

      shared_memory->games[i].is_active = !shared_memory->games[i].is_active;
      toggled = true;
      break;
    }
  }
  pthread_mutex_unlock(&shared_memory->mutex);
  return toggled;
}

bool add_player_to_shm_game(const char *room_name) {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  bool found = false;
  pthread_mutex_lock(&shared_memory->mutex);

  for (int i = 0; i < shared_memory->games_count; ++i) {
    if (std::strcmp(shared_memory->games[i].room_name, room_name) == 0) {
      shared_memory->games[i].players_in_room++;
      found = true;
      break;
    }
  }

  pthread_mutex_unlock(&shared_memory->mutex);

  return found;
}

bool remove_player_from_shm_game(const char *room_name) {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  bool found = false;
  pthread_mutex_lock(&shared_memory->mutex);

  for (int i = 0; i < shared_memory->games_count; ++i) {
    if (std::strcmp(shared_memory->games[i].room_name, room_name) == 0) {
      shared_memory->games[i].players_in_room--;
      found = true;

      if (shared_memory->games[i].players_in_room == 0) {
        if (!remove_game(room_name)) {
          perror("remove_game");
        };
      }
      break;
    }
  }

  pthread_mutex_unlock(&shared_memory->mutex);

  return found;
}

struct ClientSharedMemory {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  char command[256];
  char response[256];
  char in_room[128];
  int user_id;
};

struct RoomSharedMemory {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  char command[256];
  char response[256];
  char room_name[128];
  int turn_queue[10];
  int turn;
  int players_in_room;
  int max_players;
  char secret[10];
  int secret_length;
  bool is_active;
};

bool add_player_in_game(RoomSharedMemory *room, const std::string &user_id) {
  int id = std::stoi(user_id);

  if (room->players_in_room >= 10) {
    return false;
  }

  room->turn_queue[room->players_in_room] = id;
  room->players_in_room++;

  add_player_to_shm_game(room->room_name);
  return true;
}

bool remove_player_from_game(RoomSharedMemory *room,
                             const std::string &user_id) {
  int id = std::stoi(user_id);

  int index = -1;
  for (int i = 0; i < room->players_in_room; i++) {
    if (room->turn_queue[i] == id) {
      index = i;
      break;
    }
  }

  if (index == -1) {
    return false;
  }

  for (int i = index; i < room->players_in_room - 1; i++) {
    room->turn_queue[i] = room->turn_queue[i + 1];
  }
  room->players_in_room--;

  if (room->players_in_room == 0) {
    room->is_active = false;
  }

  if (room->turn >= room->players_in_room) {
    room->turn = 0;
  }

  return true;
}

bool next_turn(RoomSharedMemory *room) {
  if (room->players_in_room == 0) {
    return false;
  }

  room->turn = (room->turn + 1) % room->players_in_room;
  return true;
}

#endif // SHARED_MEMORY_HPP
