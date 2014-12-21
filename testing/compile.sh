#!/bin/bash
g++ server.cpp ../socket.cpp ../string.cpp ../time.cpp -o server -O -Wall -std=c++11 -ggdb #-lWs2_32
g++ client.cpp ../socket.cpp ../string.cpp ../time.cpp -o client -O -Wall -std=c++11 -ggdb #-lWs2_32
g++ udp_client.cpp ../socket.cpp ../string.cpp ../time.cpp -o udp_client -O -Wall -std=c++11 -ggdb #-lWs2_32

#g++ main.cpp ../joystick.cpp ../serial.cpp ../string.cpp ../time.cpp -o test -O -Wall -std=c++11 -ggdb
g++ test.cpp ../socket.cpp -o test -O -Wall -std=c++11 -ggdb