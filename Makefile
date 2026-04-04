CC ?= gcc
DEBUG_FLAGS ?=
CFLAGS = -Wall -Wextra -O3 -std=c99 -I/usr/local/include $(DEBUG_FLAGS)
LDFLAGS = -L/usr/local/lib -lvulkan -lsodium
GLSLC ?= glslc

TARGET = tor_vanity_vk
SRCS = main.c
OBJS = $(SRCS:.c=.o)
SHADER = shader.spv

all: $(TARGET) $(SHADER)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SHADER): shader.comp
	$(GLSLC) shader.comp -o $(SHADER)

clean:
	rm -f $(OBJS) $(TARGET) $(SHADER)
