CC = g++
CFLAGS = -Wall -Wextra -pedantic -Werror
ZMQ = `pkg-config --libs libzmq`

all: controller worker

controller: controller.cpp
	$(CC) $(CFLAGS) controller.cpp -o controller $(ZMQ)

worker: worker.cpp
	$(CC) $(CFLAGS) worker.cpp -o worker $(ZMQ)

clean:
	rm -f controller worker
