#!/bin/bash

cd build && cmake --build .

if [ "$1" == "c" ]; then
  ./client
elif [ "$1" == "s" ]; then
  ./server
else
  echo "Usage: ./run.sh [c|s]"
  echo "  c: Run the client"
  echo "  s: Run the server"
fi
