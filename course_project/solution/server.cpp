#include "func.hpp"
#include "game_logic.hpp"
#include "shared_memory.hpp"
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define SHARED_MEMORY_SIZE 1024

static int client_id_counter = 1;

template <typename T>
void send_response(T *shared_memory, const std::string &response) {
  strncpy(shared_memory->response, response.c_str(),
          sizeof(shared_memory->response) - 1);
  pthread_cond_signal(&shared_memory->cond);
}

void send_game_response(RoomSharedMemory *room_shared_memory,
                        const std::string &command, const std::string &message,
                        const std::string &turn = "-1") {
  pthread_mutex_lock(&room_shared_memory->mutex);
  strncpy(room_shared_memory->response,
          (command + ':' + turn + ':' + message).c_str(),
          sizeof(room_shared_memory->response) - 1);
  pthread_cond_broadcast(&room_shared_memory->cond);
  pthread_mutex_unlock(&room_shared_memory->mutex);
  // fmt::print("{}", (command + ':' + turn + ':' + message).c_str());
}

template <typename T> void mutex_init(T *shared_memory) {
  pthread_mutexattr_t mutexAttr;
  pthread_condattr_t condAttr;

  pthread_mutexattr_init(&mutexAttr);
  pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&shared_memory->mutex, &mutexAttr);

  pthread_condattr_init(&condAttr);
  pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
  pthread_cond_init(&shared_memory->cond, &condAttr);
}

void start_game(RoomSharedMemory *room_shared_memory) {
  send_game_response(room_shared_memory, "msg",
                     fmt::format("The game begins!\n"));

  room_shared_memory->turn = 0;
  room_shared_memory->is_active = true;
  toggle_game_state(room_shared_memory->room_name);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::string user_turn =
      std::to_string(room_shared_memory->turn_queue[room_shared_memory->turn]);
  if (room_shared_memory->players_in_room > 1) {
    send_game_response(
        room_shared_memory, "turn",
        fmt::format("The turn goes to the User {}. Wait...\n", user_turn),
        user_turn);

  } else {
    send_game_response(room_shared_memory, "turn", "turn\n", user_turn);
  }
}

void handle_game_request(RoomSharedMemory *room_shared_memory,
                         const std::string &full_command) {
  memset(room_shared_memory->command, 0, sizeof(room_shared_memory->command));
  memset(room_shared_memory->response, 0, sizeof(room_shared_memory->response));
  std::this_thread::sleep_for(std::chrono::seconds(1));

  const std::vector<std::string> &args = split(full_command, ':');
  const std::string &command = args[0];
  const std::string &user_id = args[1];

  fmt::print("Received command from client {} in room {}: {}\n", user_id,
             room_shared_memory->room_name, command);

  if (command == "leave") {
    remove_player_from_game(room_shared_memory, user_id);

    fmt::print("Client {} left the room {}.\n", user_id,
               room_shared_memory->room_name);
    if (room_shared_memory->is_active &&
        room_shared_memory->players_in_room > 0) {
      send_game_response(
          room_shared_memory, "msg",
          fmt::format("User {} left the room! Remains {} players in game!\n",
                      user_id,
                      room_shared_memory->max_players -
                          room_shared_memory->players_in_room),
          user_id);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::string user_turn = std::to_string(
          room_shared_memory->turn_queue[room_shared_memory->turn]);
      if (room_shared_memory->players_in_room > 1) {
        send_game_response(
            room_shared_memory, "turn",
            fmt::format("The turn goes to the User {}. Wait...\n", user_turn),
            user_turn);

      } else {
        send_game_response(room_shared_memory, "turn", "turn\n", user_turn);
      }

    } else {
      send_game_response(
          room_shared_memory, "msg",
          fmt::format(
              "User {} left the room! Remains {} players to start game!\n",
              user_id,
              room_shared_memory->max_players -
                  room_shared_memory->players_in_room),
          user_id);
    }

  } else if (command == "connect") {
    add_player_in_game(room_shared_memory, user_id);

    fmt::print("Client {} entered the room {}.\n", user_id,
               room_shared_memory->room_name);
    send_game_response(
        room_shared_memory, "msg",
        fmt::format("User {} entered the room! Remains {} players to "
                    "start game!\n",
                    user_id,
                    room_shared_memory->max_players -
                        room_shared_memory->players_in_room),
        user_id);

  } else if (command == "guess") {
    const std::string &num = args[2];
    const GuessResult &result = check_guess(room_shared_memory->secret, num);

    if (room_shared_memory->is_active) {

      if (result.bulls == room_shared_memory->secret_length) {
        send_game_response(
            room_shared_memory, "win",
            fmt::format(fg(fmt::color::yellow) | fmt::emphasis::bold,
                        "User {} selects the number {} and he "
                        "wins!\nThe game is over!\n",
                        user_id, num),
            user_id);
        room_shared_memory->is_active = false;
        return;

      } else {
        if (room_shared_memory->players_in_room > 1) {
          send_game_response(
              room_shared_memory, "msg",
              fmt::format("User {} selects the number {}.\n", user_id, num),
              user_id);
          std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        send_game_response(
            room_shared_memory, "msg",
            fmt::format("The server says {} {} and {} {}!\n", result.bulls,
                        (result.bulls == 1 ? "bull" : "bulls"), result.cows,
                        (result.cows == 1 ? "cow" : "cows")));
      }

      next_turn(room_shared_memory);
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::string user_turn = std::to_string(
          room_shared_memory->turn_queue[room_shared_memory->turn]);
      if (room_shared_memory->players_in_room > 1) {
        send_game_response(
            room_shared_memory, "turn",
            fmt::format("The turn goes to the User {}. Wait...\n", user_turn),
            user_turn);

      } else {
        send_game_response(room_shared_memory, "turn", "turn\n", user_turn);
      }

    } else {
      send_game_response(
          room_shared_memory, "err",
          fmt::format(fg(fmt::color::crimson) | fmt::emphasis::bold |
                          fmt::emphasis::italic,
                      "Wait for the start of the game and your turn!\n"),
          user_id);
    }

  } else {
    send_game_response(room_shared_memory, "err",
                       fmt::format(fg(fmt::color::crimson) |
                                       fmt::emphasis::bold |
                                       fmt::emphasis::italic,
                                   "Unknown command.\n"),
                       user_id);
  }
}

void handle_create(ClientSharedMemory *client_shared_memory,
                   const std::vector<std::string> &args) {
  if (args.size() < 2 || args.size() > 4) {
    send_response<ClientSharedMemory>(client_shared_memory,
                                      fmt::format(fg(fmt::color::crimson) |
                                                      fmt::emphasis::bold |
                                                      fmt::emphasis::italic,
                                                  "Invalid command format.\n"));
    return;
  }

  std::string room_name = args[1];
  int max_players =
      (args.size() > 2 && !args[2].empty()) ? std::stoi(args[2]) : 1;
  int secret_length =
      (args.size() > 3 && !args[3].empty()) ? std::stoi(args[3]) : 4;
  std::string secret = generate_secret(secret_length);

  Game new_game{};
  strncpy(new_game.room_name, room_name.c_str(),
          sizeof(new_game.room_name) - 1);
  new_game.max_players = max_players;
  new_game.players_in_room = 0;
  new_game.is_active = false;

  if (check_game_exists_in_shm(room_name.c_str())) {
    send_response<ClientSharedMemory>(client_shared_memory,
                                      fmt::format(fg(fmt::color::crimson) |
                                                      fmt::emphasis::bold |
                                                      fmt::emphasis::italic,
                                                  "Room already exists.\n"));
    return;
  } else {
    add_game(new_game);
  }

  std::string room_shm_name =
      std::string(SHARED_MEMORY_BASE) + "_room_" + room_name;
  int room_shm_fd = shm_open(room_shm_name.c_str(), O_CREAT | O_RDWR, 0666);
  if (room_shm_fd == -1) {
    perror("shm_open (game room)");
    exit(1);
  }

  if (ftruncate(room_shm_fd, sizeof(RoomSharedMemory)) == -1) {
    perror("ftruncate (game room)");
    exit(1);
  }

  RoomSharedMemory *room_shared_memory = static_cast<RoomSharedMemory *>(
      mmap(nullptr, sizeof(RoomSharedMemory), PROT_READ | PROT_WRITE,
           MAP_SHARED, room_shm_fd, 0));
  if (room_shared_memory == MAP_FAILED) {
    perror("mmap (game room)");
    exit(1);
  }

  memset(room_shared_memory, 0, sizeof(RoomSharedMemory));
  mutex_init<RoomSharedMemory>(room_shared_memory);

  pid_t pid = fork();
  if (pid == 0) {

    std::strncpy(room_shared_memory->room_name, room_name.c_str(),
                 sizeof(room_shared_memory->room_name) - 1);
    room_shared_memory->secret_length = secret_length;
    room_shared_memory->max_players = max_players;
    std::strncpy(room_shared_memory->secret, secret.c_str(),
                 sizeof(room_shared_memory->secret) - 1);
    room_shared_memory->turn = -1;
    room_shared_memory->is_active = false;

    fmt::print("Process created for room: {}, Secret: {}\n", room_name, secret);

    do {
      pthread_mutex_lock(&room_shared_memory->mutex);
      pthread_cond_wait(&room_shared_memory->cond, &room_shared_memory->mutex);
      pthread_mutex_unlock(&room_shared_memory->mutex);

      handle_game_request(room_shared_memory, room_shared_memory->command);

      memset(room_shared_memory->command, 0,
             sizeof(room_shared_memory->command));

    } while (room_shared_memory->players_in_room > 0 &&
             room_shared_memory->players_in_room !=
                 room_shared_memory->max_players);

    start_game(room_shared_memory);

    while (room_shared_memory->players_in_room > 0 &&
           room_shared_memory->is_active == true) {
      pthread_mutex_lock(&room_shared_memory->mutex);
      pthread_cond_wait(&room_shared_memory->cond, &room_shared_memory->mutex);
      pthread_mutex_unlock(&room_shared_memory->mutex);

      handle_game_request(room_shared_memory, room_shared_memory->command);

      memset(room_shared_memory->command, 0,
             sizeof(room_shared_memory->command));
    }

    remove_game(room_shared_memory->room_name);
    fmt::print("Process destroyed for room: {}\n", room_name, secret);
    munmap(room_shared_memory, sizeof(ClientSharedMemory));
    shm_unlink(room_shm_name.c_str());
    exit(0);
  } else if (pid > 0) {
    strncpy(client_shared_memory->in_room, room_name.c_str(),
            sizeof(client_shared_memory->in_room) - 1);
    send_response<ClientSharedMemory>(
        client_shared_memory,
        fmt::format(fg(fmt::color::lime_green) | fmt::emphasis::bold |
                        fmt::emphasis::italic,
                    "Room {} created successfully!\n", room_name));
    fmt::print("Parent process continuing for room {}...\n", room_name);
  } else {
    perror("fork");
    exit(1);
  }
}

void handle_enter(ClientSharedMemory *client_shared_memory,
                  const std::vector<std::string> &args) {
  if (args.size() < 2) {
    send_response<ClientSharedMemory>(client_shared_memory,
                                      fmt::format(fg(fmt::color::crimson) |
                                                      fmt::emphasis::bold |
                                                      fmt::emphasis::italic,
                                                  "Invalid command format.\n"));
    return;
  }
  std::string room_name = args[1];

  auto games = get_all_games();
  Game *game = find_game_by_name(games, room_name.c_str());

  if (game == nullptr) {
    send_response<ClientSharedMemory>(client_shared_memory,
                                      fmt::format(fg(fmt::color::crimson) |
                                                      fmt::emphasis::bold |
                                                      fmt::emphasis::italic,
                                                  "Room not found.\n"));
    return;
  }

  if (game->players_in_room >= game->max_players) {
    send_response<ClientSharedMemory>(client_shared_memory,
                                      fmt::format(fg(fmt::color::crimson) |
                                                      fmt::emphasis::bold |
                                                      fmt::emphasis::italic,
                                                  "Room is full.\n"));
    return;
  }

  strncpy(client_shared_memory->in_room, room_name.c_str(),
          sizeof(client_shared_memory->in_room) - 1);
  fmt::print("Client {} entered the room {}.\n",
             std::to_string(client_shared_memory->user_id), room_name);
  send_response<ClientSharedMemory>(
      client_shared_memory,
      fmt::format(fg(fmt::color::lime_green) | fmt::emphasis::bold |
                      fmt::emphasis::italic,
                  "Entered the room successfully.\n"));
}

void handle_find(ClientSharedMemory *client_shared_memory) {
  std::string rooms;

  for (const Game &game : get_all_games()) {
    if (game.players_in_room < game.max_players && !game.is_active) {
      rooms += fmt::format(
          fg(fmt::color::green), "- {} ({}/{}) - {}\n",
          std::string(game.room_name), std::to_string(game.players_in_room),
          std::to_string(game.max_players),
          game.is_active ? "in progress" : "waiting for the players");
    } else {
      rooms += fmt::format(
          fg(fmt::color::red), "- {} ({}/{}) - {}\n",
          std::string(game.room_name), std::to_string(game.players_in_room),
          std::to_string(game.max_players),
          game.is_active ? "in progress" : "waiting for the players");
    }
  }

  if (rooms.empty()) {
    send_response<ClientSharedMemory>(
        client_shared_memory,
        fmt::format(fg(fmt::color::crimson) | fmt::emphasis::bold |
                        fmt::emphasis::italic,
                    "No available rooms found.\n"));
  } else {
    send_response<ClientSharedMemory>(
        client_shared_memory,
        fmt::format(
            "\n{}\n{}\n",
            fmt::format(fg(fmt::color::antique_white) | fmt::emphasis::bold,
                        "Rooms:"),
            fmt::format(rooms)));
  }
}

void handle_client_request(ClientSharedMemory *client_shared_memory) {
  std::string command = client_shared_memory->command;
  std::vector<std::string> args = split(command);

  if (args.empty()) {
    send_response<ClientSharedMemory>(client_shared_memory,
                                      fmt::format(fg(fmt::color::crimson) |
                                                      fmt::emphasis::bold |
                                                      fmt::emphasis::italic,
                                                  "Invalid command.\n"));
    return;
  }

  const std::string &cmd = args[0];
  if (cmd == "create") {
    handle_create(client_shared_memory, args);
  } else if (cmd == "enter") {
    handle_enter(client_shared_memory, args);
  } else if (cmd == "find") {
    handle_find(client_shared_memory);
  } else {
    send_response<ClientSharedMemory>(client_shared_memory,
                                      fmt::format(fg(fmt::color::crimson) |
                                                      fmt::emphasis::bold |
                                                      fmt::emphasis::italic,
                                                  "Unknown command.\n"));
  }
}

void handle_id(SharedMemory *shared_memory) {
  int client_id = client_id_counter++;

  std::string client_id_str = std::to_string(client_id);
  fmt::print("Assigned client ID: {}\n", client_id_str);

  pid_t pid = fork();
  if (pid == 0) {
    std::string client_shm_name =
        std::string(SHARED_MEMORY_BASE) + '_' + client_id_str;

    int client_shm_fd =
        shm_open(client_shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (client_shm_fd == -1) {
      perror("shm_open (client)");
      exit(1);
    }

    if (ftruncate(client_shm_fd, sizeof(ClientSharedMemory)) == -1) {
      perror("ftruncate (client)");
      close(client_shm_fd);
      exit(1);
    }

    ClientSharedMemory *client_shared_memory =
        static_cast<ClientSharedMemory *>(
            mmap(nullptr, sizeof(ClientSharedMemory), PROT_READ | PROT_WRITE,
                 MAP_SHARED, client_shm_fd, 0));
    if (client_shared_memory == MAP_FAILED) {
      perror("mmap (client)");
      exit(1);
    }

    memset(client_shared_memory, 0, sizeof(ClientSharedMemory));
    client_shared_memory->user_id = client_id;

    mutex_init<ClientSharedMemory>(client_shared_memory);
    fmt::print("Process created for client ID: {}\n", client_id_str);

    send_response<SharedMemory>(shared_memory, client_id_str);

    while (true) {
      pthread_mutex_lock(&client_shared_memory->mutex);
      pthread_cond_wait(&client_shared_memory->cond,
                        &client_shared_memory->mutex);

      std::string command = client_shared_memory->command;
      fmt::print("Received command from client {}: {}\n", client_id, command);

      if (command == "exit") {
        fmt::print("Client {} disconnected.\n", client_id_str);
        pthread_mutex_unlock(&client_shared_memory->mutex);
        break;
      }

      handle_client_request(client_shared_memory);

      memset(client_shared_memory->command, 0,
             sizeof(client_shared_memory->command));
      pthread_mutex_unlock(&client_shared_memory->mutex);
    }

    munmap(client_shared_memory, sizeof(ClientSharedMemory));
    shm_unlink(client_shm_name.c_str());
    exit(0);
  } else if (pid > 0) {
    fmt::print("Parent process continuing for client {}...\n", client_id);
  } else {
    perror("fork");
    exit(1);
  }
}

void handle_id_request(SharedMemory *shared_memory) {
  std::string command = shared_memory->command;

  if (command == "get_id") {
    handle_id(shared_memory);
  } else {
    send_response<SharedMemory>(shared_memory,
                                fmt::format(fg(fmt::color::crimson) |
                                                fmt::emphasis::bold |
                                                fmt::emphasis::italic,
                                            "Unknown command!\n"));
  }
}

int main() {
  shm_unlink(SHARED_MEMORY_BASE);
  int shm_fd = shm_open(SHARED_MEMORY_BASE, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  if (ftruncate(shm_fd, sizeof(SharedMemory)) == -1) {
    perror("ftruncate");
    close(shm_fd);
    exit(1);
  }

  SharedMemory *shared_memory = static_cast<SharedMemory *>(
      mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED,
           shm_fd, 0));
  if (shared_memory == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    exit(1);
  }

  memset(shared_memory, 0, sizeof(SharedMemory));
  mutex_init<SharedMemory>(shared_memory);

  fmt::print("Server is running. Waiting for commands...\n");

  while (true) {
    pthread_mutex_lock(&shared_memory->mutex);
    pthread_cond_wait(&shared_memory->cond, &shared_memory->mutex);

    fmt::print("Received command: {}\n", shared_memory->command);
    handle_id_request(shared_memory);

    memset(shared_memory->command, 0, sizeof(shared_memory->command));
    pthread_mutex_unlock(&shared_memory->mutex);
  }

  munmap(shared_memory, sizeof(*shared_memory));
  shm_unlink(SHARED_MEMORY_BASE);

  return 0;
}
