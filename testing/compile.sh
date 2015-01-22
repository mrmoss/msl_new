#!/bin/bash
g++ server.cpp ../socket.cpp ../string.cpp ../time.cpp -o server -O -Wall -std=c++11 -ggdb #-lWs2_32
g++ client.cpp ../socket.cpp ../string.cpp ../time.cpp -o client -O -Wall -std=c++11 -ggdb #-lWs2_32
g++ udp_client.cpp ../socket.cpp ../string.cpp ../time.cpp -o udp_client -O -Wall -std=c++11 -ggdb #-lWs2_32
#g++ test.cpp ../socket.cpp -o test -O -Wall -std=c++11 -ggdb
#g++ crypto_test.cpp ../crypto.cpp ../string.cpp -o crypto_test -O -Wall -std=c++11 -lcrypto -ggdb #-lgdi32
#g++ vec_test.cpp -o vec_test -O -Wall -std=c++11#g++ file_test.cpp ../file.cpp -o file_test -O -Wall -std=c++11
#g++ file_test.cpp ../file.cpp -o file_test -O -Wall -std=c++11
#g++ webserver_test.cpp ../mongoose/mongoose.c ../time.cpp ../webserver.cpp -o webserver_test -O -Wall -std=c++11 -lpthread
#g++ serial_test.cpp ../serial.cpp -o serial_test -O -Wall -std=c++11 -ggdb
#g++ joystick_test.cpp ../joystick.cpp ../time.cpp -o joystick_test -O -Wall -std=c++11 -ggdb
