CC ?= gcc
DEBUG_FLAGS ?=
CFLAGS = -Wall -Wextra -O3 -std=c99 -D_POSIX_C_SOURCE=199309L -pthread -I/usr/local/include $(DEBUG_FLAGS)
LDFLAGS = -L/usr/local/lib -lvulkan -lsodium -pthread
GLSLC ?= glslc

TARGET = tor_vanity_vk
SRCS = main.c fe.c ge.c sc.c
OBJS = $(SRCS:.c=.o)
SHADER = shader.spv

all: $(TARGET) $(SHADER)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SHADER): shader.comp
	$(GLSLC) -O shader.comp -o $(SHADER)

clean:
	rm -f $(OBJS) $(TARGET) $(SHADER)
