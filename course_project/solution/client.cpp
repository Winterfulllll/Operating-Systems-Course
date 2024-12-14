#include "func.hpp"
#include "shared_memory.hpp"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <fcntl.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <pthread.h>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#define SHARED_MEMORY_SIZE 1024
std::string SHARED_MEMORY = "";
std::string SHARED_ROOM_MEMORY = "";
std::string CURRENT_USER_ID_STRING = "";

std::atomic<bool> client_in_game(false);
std::atomic<bool> client_turn(false);

void print_commands() {
  fmt::print(fg(fmt::color::antique_white) | fmt::emphasis::bold,
             "\nAvailable commands:\n");
  fmt::print("- ");
  fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold,
             "create <room_name> <players_count> <digits_count>");
  fmt::print(fmt::emphasis::italic, ": create a game room;\n");
  fmt::print("- ");
  fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold,
             "enter <room_name>");
  fmt::print(fmt::emphasis::italic, ": join a game room;\n");
  fmt::print("- ");
  fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold, "find");
  fmt::print(fmt::emphasis::italic, ": find a free game room;\n");
  fmt::print("- ");
  fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold, "exit");
  fmt::print(fmt::emphasis::italic, ": exit the game;\n\n");
  ;
}

void print_greeting() {
  fmt::print(fg(fmt::color::antique_white) | fmt::emphasis::bold,
             "\n{}  Welcome to the Bulls and Cows game!\n{}\n",
             generate_separator(39), generate_separator(39));
  print_commands();
}

void print_room_greeting(std::string room_name) {
  fmt::print(fg(fmt::color::antique_white) | fmt::emphasis::bold,
             "\n===== Welcome to {} room! =====\n\n", room_name);
}

void get_id() {
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open (get_id)");
    exit(1);
  }

  auto *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap (get_id)");
    exit(1);
  }

  pthread_mutex_lock(&shared_memory->mutex);
  strncpy(shared_memory->command, "get_id", sizeof(shared_memory->command) - 1);
  pthread_cond_signal(&shared_memory->cond);
  pthread_cond_wait(&shared_memory->cond, &shared_memory->mutex);

  CURRENT_USER_ID_STRING = shared_memory->response;
  SHARED_MEMORY =
      std::string(SHARED_MEMORY_BASE) + '_' + CURRENT_USER_ID_STRING;

  pthread_mutex_unlock(&shared_memory->mutex);
  munmap(shared_memory, sizeof(SharedMemory));
  close(shm_fd);
}

template <typename T>
void send_request_and_wait_response(const std::string &request,
                                    T *shared_memory) {
  pthread_mutex_lock(&shared_memory->mutex);
  strncpy(shared_memory->command, request.c_str(),
          sizeof(shared_memory->command) - 1);
  pthread_cond_signal(&shared_memory->cond);
  pthread_cond_wait(&shared_memory->cond, &shared_memory->mutex);
  std::string response(shared_memory->response);
  memset(shared_memory->response, 0, sizeof(shared_memory->response));
  pthread_mutex_unlock(&shared_memory->mutex);
  fmt::print(response);
}

void send_exit_request(const std::string &request,
                       ClientSharedMemory *client_shared_memory) {
  pthread_mutex_lock(&client_shared_memory->mutex);
  strncpy(client_shared_memory->command, request.c_str(),
          sizeof(client_shared_memory->command) - 1);
  pthread_cond_signal(&client_shared_memory->cond);
  pthread_mutex_unlock(&client_shared_memory->mutex);
  fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold |
                 fmt::emphasis::italic,
             "\nGoodbye! Have a nice day!\n");
}

void send_leave_request(RoomSharedMemory *client_shared_memory) {
  std::string request = "leave:" + CURRENT_USER_ID_STRING;
  pthread_mutex_lock(&client_shared_memory->mutex);
  strncpy(client_shared_memory->command, request.c_str(),
          sizeof(client_shared_memory->command) - 1);
  pthread_cond_broadcast(&client_shared_memory->cond);
  pthread_mutex_unlock(&client_shared_memory->mutex);

  fmt::print(
      fg(fmt::color::lime_green) | fmt::emphasis::bold | fmt::emphasis::italic,
      "\nYou left the game room {}!\n\n", client_shared_memory->room_name);
}

void send_connection_request(RoomSharedMemory *room_shared_memory) {
  std::string request = "connect:" + CURRENT_USER_ID_STRING;
  pthread_mutex_lock(&room_shared_memory->mutex);
  strncpy(room_shared_memory->command, request.c_str(),
          sizeof(room_shared_memory->command) - 1);
  pthread_cond_broadcast(&room_shared_memory->cond);
  pthread_mutex_unlock(&room_shared_memory->mutex);
}

void send_guess_request(const std::string &guess,
                        RoomSharedMemory *room_shared_memory) {
  client_turn.store(false);
  std::string request = "guess:" + CURRENT_USER_ID_STRING + ':' + guess;
  pthread_mutex_lock(&room_shared_memory->mutex);
  memset(room_shared_memory->response, 0, sizeof(room_shared_memory->response));
  strncpy(room_shared_memory->command, request.c_str(),
          sizeof(room_shared_memory->command) - 1);
  pthread_cond_broadcast(&room_shared_memory->cond);
  pthread_mutex_unlock(&room_shared_memory->mutex);
}

std::string game_wait_and_get_message(RoomSharedMemory *room_shared_memory) {
  std::string response(room_shared_memory->response);
  // fmt::print("{}\n", response);
  if (!response.size()) {
    return "";
  }

  std::vector<std::string> response_args = split(response, ':');
  const std::string &command = response_args[0];
  const std::string &user_id = response_args[1];

  if (client_in_game.load()) {
    if (command == "msg") {
      if (user_id == "-1" || user_id != CURRENT_USER_ID_STRING) {
        fmt::print("\r\033[K");
        fmt::print(response_args[2]);
        fmt::print("> ");
        std::cout.flush();
      } else if (user_id == CURRENT_USER_ID_STRING) {
        return "";
      }
    } else if (command == "err") {
      if (user_id == CURRENT_USER_ID_STRING) {
        fmt::print("\r\033[K");
        fmt::print(response_args[2]);
        fmt::print("> ");
        std::cout.flush();
      }
      return "";
    } else if (command == "turn") {
      if (room_shared_memory->is_active && user_id == CURRENT_USER_ID_STRING) {
        if (user_id == CURRENT_USER_ID_STRING) {
          if (room_shared_memory->players_in_room > 1) {
            fmt::print("\r\033[K");
            fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold,
                       "Your turn! Enter your guess!\n");
            fmt::print("> ");
            std::cout.flush();
          } else {
            fmt::print("\r\033[K");
            fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold,
                       "Enter your guess!\n");
            fmt::print("> ");
            std::cout.flush();
          }
          return "turn";
        } else {
          fmt::print("\r\033[K");
          fmt::print(response_args[2]);
          fmt::print("> ");
          std::cout.flush();
        }
      }
      return "";
    } else if (command == "win") {
      if (user_id == CURRENT_USER_ID_STRING) {
        fmt::print("\r\033[K");
        fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold,
                   "Congratulations, you have won! Well played!\n Press ENTER to "
                   "exit the lobby!");
        std::cout.flush();
      } else {
        fmt::print("\r\033[K");
        fmt::print(response_args[2]);
        fmt::print(fg(fmt::color::lime_green) | fmt::emphasis::bold,
                   "Press ENTER to exit the lobby!");
        std::cout.flush();
      }
    } else {
      fmt::print("\r\033[K");
      fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold |
                     fmt::emphasis::italic,
                 "\nThe server response is not expected: {}\n", response);
      fmt::print("> ");
      std::cout.flush();
      return "";
    }
  }

  return command;
}

void *server_message_listener(void *arg) {
  RoomSharedMemory *room_shared_memory = static_cast<RoomSharedMemory *>(arg);

  while (client_in_game.load()) {
    pthread_mutex_lock(&room_shared_memory->mutex);
    pthread_cond_wait(&room_shared_memory->cond, &room_shared_memory->mutex);
    std::string server_response = game_wait_and_get_message(room_shared_memory);
    pthread_mutex_unlock(&room_shared_memory->mutex);

    if (!server_response.empty()) {
      if (server_response == "turn") {
        client_turn.store(true);
      } else if (server_response == "win") {
        client_in_game.store(false);
        break;
      }
    }
  }

  return nullptr;
}

void enter_room(std::string room_name) {
  print_room_greeting(room_name);

  SHARED_ROOM_MEMORY = std::string(SHARED_MEMORY_BASE) + "_room_" + room_name;
  int shm_fd = shm_open(SHARED_ROOM_MEMORY.c_str(), O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  RoomSharedMemory *room_shared_memory = static_cast<RoomSharedMemory *>(
      mmap(nullptr, sizeof(RoomSharedMemory), PROT_READ | PROT_WRITE,
           MAP_SHARED, shm_fd, 0));
  if (room_shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  client_in_game.store(true);
  pthread_t listener_thread;
  if (pthread_create(&listener_thread, nullptr, server_message_listener,
                     room_shared_memory) != 0) {
    perror("pthread_create");
    exit(1);
  }

  send_connection_request(room_shared_memory);

  std::string request;
  std::cout << "> ";
  while (std::getline(std::cin, request)) {
    if (client_in_game.load()) {
      if (request == "leave") {
        send_leave_request(room_shared_memory);
        break;
      } else {
        if (client_turn.load()) {
          if (is_invalid_num(request, room_shared_memory->secret_length)) {
            fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                       "Your guess should be a number of length {}!\n",
                       std::to_string(room_shared_memory->secret_length));
            std::cout << "> ";
            continue;
          } else {
            send_guess_request(request, room_shared_memory);
            client_turn.store(false);
          }
        } else if (room_shared_memory->is_active) {
          fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                     "Invalid input. Wait for your turn or enter \"leave\" to "
                     "leave the game room!\n");
          std::cout << "> ";
          continue;
        } else {
          fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,
                     "Invalid input. Wait until the room is full and the game "
                     "starts, or enter \"leave\" to leave the game room!\n");
          std::cout << "> ";
          continue;
        }
      }
    } else {
      break;
    }
  }

  client_in_game.store(false);
  pthread_join(listener_thread, nullptr);

  SHARED_ROOM_MEMORY = "";
  munmap(room_shared_memory, sizeof(RoomSharedMemory));
  close(shm_fd);
}

int main() {
  std::cout.setf(std::ios::unitbuf);
  print_greeting();
  get_id();

  int shm_fd = shm_open(SHARED_MEMORY.c_str(), O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  ClientSharedMemory *shared_memory = static_cast<ClientSharedMemory *>(
      mmap(nullptr, sizeof(ClientSharedMemory), PROT_READ | PROT_WRITE,
           MAP_SHARED, shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  shared_memory->user_id = std::stoi(CURRENT_USER_ID_STRING);
  std::string request;
  std::vector<std::string> request_args;

  while (std::cout << "> " && std::getline(std::cin, request)) {
    if (request == "") {
      fmt::print(
          fg(fmt::color::crimson) | fmt::emphasis::bold,
          "Error: Unknown command. Type 'help' for a list of commands.\n");
      continue;
    }
    std::vector<std::string> request_args = split(request);

    if (request_args[0] == "create") {
      if (is_invalid_create(request_args)) {
        continue;
      }
    } else if (request_args[0] == "enter") {
      if (is_invalid_enter(request_args)) {
        continue;
      }
    } else if (request == "find") {
    } else if (request == "help") {
      print_commands();
      continue;
    } else if (request == "exit") {
      send_exit_request(request, shared_memory);
      break;
    } else {
      fmt::print(
          fg(fmt::color::crimson) | fmt::emphasis::bold,
          "Error: Unknown command. Type 'help' for a list of commands.\n");
      continue;
    }

    send_request_and_wait_response<ClientSharedMemory>(request, shared_memory);

    if (!std::string(shared_memory->in_room).empty()) {
      enter_room(std::string(shared_memory->in_room));
      memset(shared_memory->in_room, 0, sizeof(shared_memory->in_room));
    }
  }

  munmap(shared_memory, sizeof(ClientSharedMemory));
  close(shm_fd);
}
