CC := /opt/rcc-1.3.2-gcc/bin/sparc-gaisler-rtems5-gcc
CFLAGS := -g -mcpu=leon3 -qbsp=leon3
SRC := main.c PUS_3.c General_Functions.c PUS.c Space_Packet_Protocol.c COBS.c PUS_1_service.c PUS_17_service.c
OUT := main

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS)

clean:
	rm -f $(OUT)
