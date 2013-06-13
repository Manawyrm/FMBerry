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
        $(RM) -f *.o $(TARGET_DAEMON)

install:
        install fmberry.conf /etc/fmberry.conf
        install fmberryd /usr/local/bin
        install ctlfmberry /usr/local/bin
        install fmberry /etc/init.d

uninstall:
        $(RM) /usr/local/bin/fmberryd
        $(RM) /usr/local/bin/ctlfmberry
        $(RM) /etc/init.d/fmberry
