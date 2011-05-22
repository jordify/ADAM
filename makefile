hamz: 
	g++ -Wall -c hamz.cpp
	g++ -lzmq -lrt -o hamz hamz.o
ham: 
	g++ -Wall -c ham.cpp
	g++ -lrt -o ham ham.o
clean:
	rm *.o hamz
