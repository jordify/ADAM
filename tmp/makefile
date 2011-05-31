ham: 
	g++ -Wall -c hamz.cpp
	g++ -lrt -lzmq -o ham hamz.o
tcpLat:
	g++ -Wall -c hamTCPLat.cpp
	g++ -lrt -lzmq -o tcpLat hamTCPLat.o
tcpThr:
	g++ -Wall -c hamTCPThr.cpp
	g++ -lrt -lzmq -o tcpThr hamTCPThr.o
zmqThr:
	g++ -Wall -c hamZMQThr.cpp
	g++ -lrt -lzmq -o zmqThr hamZMQThr.o
zmqLat:
	g++ -Wall -c hamZMQLat.cpp
	g++ -lrt -lzmq -o zmqLat hamZMQLat.o
ftLat:
	g++ -Wall -c hamFTLat.cpp
	g++ -lrt -lzmq -lz -o ftLat hamFTLat.o
ftThr:
	g++ -Wall -c hamFTThr.cpp
	g++ -lrt -lzmq -lz -o ftThr hamFTThr.o
clean:
	rm *.o ham
cleanLat:
	rm *.o tcpLat
cleanThr:
	rm *.o tcpThr
cleanzThr:
	rm *.o zmqThr
cleanzLat:
	rm *.o zmqLat
cleanfLat:
	rm *.o ftLat
cleanfThr:
	rm *.o ftThr
