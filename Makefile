TARGET_DAEMON=fmberryd
SRC=fmberry.c ns741.o i2c.o
SRC_DAEMON=fmberryd.c ns741.o i2c.o

all: fmberryd

fmberryd: ns741
	$(CC) -o $(TARGET_DAEMON) $(SRC_DAEMON) -l bcm2835 -l pthread -l confuse

ns741: i2c
	$(CC) -c -o ns741.o ns741.c

i2c:
	$(CC) -c -o i2c.o i2c.c

clean:
	rm -f *.o

install:
	rm -f *.o
	cp fmberry.conf /etc/fmberry.conf
	cp fmberryd /usr/local/bin
	cp ctlfmberry /usr/local/bin
	cp fmberry /etc/init.d