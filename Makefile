CC = gcc

CFLAGS = -Wall -Werror -g

TARGET = main

SOURCES = $(shell find . -name "*.c")

OBJECTS = $(SOURCES:.c=.o)

ALL: $(TARGET)

$(TARGET) : $(OBJECTS)
		$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(TARGET)