#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <zmq.hpp>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: ./worker <endpoint>" << std::endl;
    return 1;
  }

  zmq::context_t ctx;
  zmq::socket_t socket(ctx, zmq::socket_type::rep);
  socket.bind(argv[1]);

  while (true) {
    zmq::message_t request;
    if (!socket.recv(request, zmq::recv_flags::none))
      continue;

    std::string command = request.to_string();
    std::istringstream iss(command);

    std::string cmd;
    iss >> cmd;

    if (cmd == "ping") {
      socket.send(zmq::buffer("Ok"), zmq::send_flags::none);

    } else if (cmd == "exec") {
      int n;
      iss >> n;
      int sum = 0;
      for (int i = 0; i < n; ++i) {
        int num;
        iss >> num;
        sum += num;
      }

      socket.send(zmq::buffer(std::to_string(sum)), zmq::send_flags::none);

    } else {
      socket.send(zmq::buffer("Error: Unknown command"), zmq::send_flags::none);
    }
  }

  return 0;
}
