CC=g++
CFLAGS=-Wall -Wextra
LDFLAGS=-lzmq

all: thr lat

thr: remote_thr local_thr

lat: remote_lat local_lat

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f remote_lat remote_thr local_lat local_thr
