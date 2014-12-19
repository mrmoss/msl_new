#!/bin/bash
g++ main.cpp ../socket.cpp ../string.cpp ../time.cpp -o test -O -Wall -std=c++11 -ggdb
g++ client.cpp ../socket.cpp ../string.cpp ../time.cpp -o client -O -Wall -std=c++11 -ggdb

#g++ main.cpp ../joystick.cpp ../serial.cpp ../string.cpp ../time.cpp -o test -O -Wall -std=c++11 -ggdb