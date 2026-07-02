CC ?= cc
CFLAGS = -Wall -Wextra -O3 -std=c99 -D_POSIX_C_SOURCE=199309L -pthread -I/usr/local/include
LDFLAGS = -L/usr/local/lib -lvulkan -lsodium -pthread

TARGET = tor_vanity_vk
SRCS = main.c fe.c ge.c sc.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET) shaders

$(TARGET): $(OBJS)
	@echo "  LD      $@"
	@$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) -c $< -o $@

shaders: shader.comp
	@echo "  SHLC    shader.comp"
	@./build_shader.sh

clean:
	@echo "  CLEAN"
	@rm -f $(OBJS) $(TARGET) *.spv
