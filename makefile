all: 
#	gcc -lzmq -o zserver zmqTestServer.c
#	gcc -lzmq -o zclient zmqTestClient.c
#	gcc -lzmq -o server tcpTestServer.c
#	gcc -lzmq -o client tcpTestClient.c
	g++ -Wall -c ham.cpp
clean:
#	rm client server zclient zserver *.o
	rm *.o
