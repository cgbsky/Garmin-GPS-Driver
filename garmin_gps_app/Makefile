CC = gcc

#Source := gps_dump.c
#Source := gps_dump_.c

Source := nmeagen.c	\
	garminusb2nmea.c

Object := $(patsubst %.c,%.o,$(Source))

all: $(Object)
	$(CC) -o gps $(Object) -lm -lgps

clean:
	rm -rf *.o gps
