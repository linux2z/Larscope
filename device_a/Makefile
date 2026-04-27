CC = gcc
CFLAGS = -Wall -Wextra -O2 -g -I../shared $(shell pkg-config --cflags json-c gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-app-1.0)
LDFLAGS = -lpthread $(shell pkg-config --libs json-c gstreamer-1.0 gstreamer-rtsp-server-1.0 gstreamer-app-1.0) -lgpiod

TARGET = larscope-a

SHARED_SRCS = \
	../shared/event_bus.c \
	../shared/module.c \
	../shared/config.c \
	../shared/logger.c

APP_SRCS = \
	main.c \
	gpio_input.c \
	status_led.c \
	illum_led.c \
	thermal_mon.c \
	camera.c \
	recording.c \
	streaming.c \
	network_mgr.c \
	storage.c \
	cmd_server.c

SRCS = $(SHARED_SRCS) $(APP_SRCS)
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
