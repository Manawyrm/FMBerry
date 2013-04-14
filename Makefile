TARGET_DAEMON=fmberryd
SRC=fmberry.c ns741.o i2c.o
SRC_DAEMON=fmberryd.c ns741.o i2c.o

all: fmberryd

fmberryd: ns741
	$(CC) -o $(TARGET_DAEMON) $(SRC_DAEMON) -l bcm2835

ns741: i2c
	$(CC) -c -o ns741.o ns741.c

i2c:
	$(CC) -c -o i2c.o i2c.c

clean:
	rm -f *.o
