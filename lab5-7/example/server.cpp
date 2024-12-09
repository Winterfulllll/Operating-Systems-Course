#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <zmq.hpp>

int main() {
  using namespace std::chrono_literals;

  // initialize the zmq context with a single IO thread
  zmq::context_t context{1};

  // construct a REP (reply) socket and bind to interface
  zmq::socket_t socket{context, zmq::socket_type::rep};
  socket.bind("tcp://*:5555");

  // prepare some static data for responses
  const std::string data{"World"};

  for (;;) {
    zmq::message_t request;

    // receive a request from client
    zmq::recv_result_t res = socket.recv(request, zmq::recv_flags::none);
    if (!res.has_value()) {
      std::cerr << "Error receiving message: " << zmq_strerror(zmq_errno())
                << std::endl;
      return 1;
    }
    std::cout << "Received " << request.to_string() << std::endl;

    // simulate work
    std::this_thread::sleep_for(1s);

    // send the reply to the client
    socket.send(zmq::buffer(data), zmq::send_flags::none);
  }

  return 0;
}
