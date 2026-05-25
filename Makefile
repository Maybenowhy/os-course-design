CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -Isrc
TARGET = os_course_design.exe
SRCS = src/main.c src/scheduler.c src/memory.c src/sync_demo.c src/filesystem.c src/platform_thread.c
OBJS = $(SRCS:.c=.o)

ifeq ($(OS),Windows_NT)
LDFLAGS =
else
TARGET = os_course_design
LDFLAGS = -pthread
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
ifeq ($(OS),Windows_NT)
	del /Q src\*.o $(TARGET) 2>NUL || exit 0
else
	rm -f src/*.o $(TARGET)
endif
