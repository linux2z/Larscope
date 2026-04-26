CC = gcc
CFLAGS = -Wall -Wextra -O2 -g $(shell pkg-config --cflags gstreamer-1.0)
LDFLAGS = -lpthread $(shell pkg-config --libs gstreamer-1.0)

TARGET = larscope-b

SRCS = main.c stream_client.c cmd_client.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
