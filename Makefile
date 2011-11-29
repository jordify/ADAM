all:
	cd src && make local && make
	cd perf/ZMQperf && make

clean:
	cd src && make pristene
	cd perf/ZMQperf && make clean
