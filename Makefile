C=gcc
CFLAGS= -O3 -Wall -g
LIBS= -lm
AR=ar

ALLBIN=getbno055

all: ${ALLBIN}

clean:
	rm -f *.o ${ALLBIN}

getbno055: i2c_bno055.o getbno055.o
	$(CC) i2c_bno055.o getbno055.o -o getbno055 ${LIBS}

