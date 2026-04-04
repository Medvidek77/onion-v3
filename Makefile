CC ?= gcc
CFLAGS = -Wall -Wextra -O3 -std=c99 -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lOpenCL -lsodium

TARGET = tor_vanity_ocl
SRCS = main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
