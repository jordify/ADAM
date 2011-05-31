ham: 
	g++ -Wall -c hamz.cpp
	g++ -lrt -lzmq -o ham hamz.o
clean:
	rm *.o ham
