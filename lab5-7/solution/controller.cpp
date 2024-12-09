#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <signal.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include <zmq.hpp>

using namespace std::chrono_literals;

struct Node {
  int id;
  pid_t pid;
  std::string endpoint;
  bool is_alive = true;
};

class ProcessManager {
private:
  std::map<int, Node> nodes;
  std::mutex node_mutex;
  zmq::context_t ctx;
  zmq::socket_t hb_socket;

  std::thread hb_thread;
  bool hb_running = false;
  int hb_interval = 0;

  void heartbit_monitor() {
    while (hb_running) {
      std::this_thread::sleep_for(std::chrono::milliseconds(hb_interval * 4));
      std::lock_guard<std::mutex> lock(node_mutex);

      std::set<int> dead_nodes;
      for (auto &[id, node] : nodes) {
        zmq::socket_t socket(ctx, zmq::socket_type::req);
        socket.set(zmq::sockopt::rcvtimeo, 1000);

        try {
          socket.connect(node.endpoint);
          socket.send(zmq::buffer("ping "), zmq::send_flags::none);

          zmq::message_t response;
          if (socket.recv(response, zmq::recv_flags::none)) {
            std::string reply = static_cast<char *>(response.data());

            if (reply != "Ok") {
              node.is_alive = false;
            } else {
              node.is_alive = true;
            }
          } else {
            node.is_alive = false;
            dead_nodes.insert(id);
          }
        } catch (...) {
          node.is_alive = false;
          dead_nodes.insert(id);
        }
      }

      for (int id : dead_nodes) {
        std::cout << "\r" << std::string(20, ' ') << "\r";
        std::cout << "Heartbit: node " << id << " is unavailable now\n";
        std::cout << "> ";
        std::cout.flush();
      }
    }
  }

public:
  ProcessManager() : ctx(1), hb_socket(ctx, zmq::socket_type::req) {}

  void create_node(int id) {
    std::lock_guard<std::mutex> lock(node_mutex);

    if (nodes.find(id) != nodes.end()) {
      std::cout << "Error: Already exists\n";
      return;
    }

    pid_t pid = fork();
    if (pid == -1) {
      std::cout << "Error: Could not fork\n";
      return;
    }

    std::string endpoint = "ipc:///tmp/node_" + std::to_string(id);

    if (pid == 0) {
      const char *args[] = {"./worker", endpoint.c_str(), nullptr};
      execv(args[0], const_cast<char *const *>(args));
      exit(1);
    }

    Node node{id, pid, endpoint, true};
    nodes[id] = node;
    std::cout << "Ok: " << pid << std::endl;
  }

  void execute_command(int id, const std::string &command) {
    std::lock_guard<std::mutex> lock(node_mutex);

    auto it = nodes.find(id);
    if (it == nodes.end()) {
      std::cout << "Error: Node was not found\n";
      return;
    }

    Node &node = it->second;
    if (!node.is_alive) {
      std::cout << "Error: Node is unavailable\n";
      return;
    }

    zmq::socket_t socket(ctx, zmq::socket_type::req);
    socket.connect(node.endpoint);

    socket.send(zmq::buffer("exec " + command), zmq::send_flags::none);
    zmq::message_t response;
    if (socket.recv(response, zmq::recv_flags::none)) {
      std::cout << "Ok:" << id << ": " << response.to_string() << std::endl;
    } else {
      std::cout << "Error: Node did not respond\n";
    }
  }

  void start_heartbit(int interval) {
    if (hb_running) {
      std::cout << "Error: Heartbit already running\n";
      return;
    }

    hb_interval = interval;
    hb_running = true;
    hb_thread = std::thread(&ProcessManager::heartbit_monitor, this);
    std::cout << "Ok\n";
  }

  void stop_heartbit() {
    if (!hb_running) {
      std::cout << "Error: Heartbit is not running\n";
      return;
    }

    hb_running = false;
    if (hb_thread.joinable()) {
      hb_thread.join();
    }
    std::cout << "Ok\n";
  }

  ~ProcessManager() {
    hb_running = false;
    if (hb_thread.joinable()) {
      hb_thread.join();
    }
    for (auto &[id, node] : nodes) {
      kill(node.pid, SIGKILL);
    }
  }
};

int main() {
  ProcessManager manager;
  std::string command;

  while (std::cout << "> " && std::getline(std::cin, command)) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "create") {
      int id;
      iss >> id;
      manager.create_node(id);

    } else if (cmd == "exec") {
      int id;
      iss >> id;
      std::string params;
      std::getline(iss, params);
      manager.execute_command(id, params);

    } else if (cmd == "heartbit") {
      int interval;
      iss >> interval;
      manager.start_heartbit(interval);

    } else if (cmd == "stop_heartbit") {
      manager.stop_heartbit();

    } else if (cmd == "exit") {
      break;

    } else {
      std::cout << "Error: Unknown command\n";
    }
  }

  return 0;
}
