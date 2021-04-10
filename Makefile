# VARIABLES

FILES = $(wildcard src/*.c) $(wildcard lib/*.c)
HEADERS = $(wildcard include/*.h)
OBJS = $(FILES:.c=.o)
TARGET = pairwars   # change target to change executible name

CXX = gcc

CFLAGS = -Iinclude
LFLAGS =


.PHONY: all
all: $(TARGET)

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LFLAGS)

%.o:	%.c
	$(CXX) -c $< -o $@ $(CFLAGS)


.PHONY: run
run:
	./$(TARGET)


.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)


.PHONY: vim
vim: 
	vim $(FILES) $(HEADERS)


.PHONY: project
project: 
	mkdir src lib include
	touch ./src/main.c
	echo "#include <stdio.h> \n\nint main() { \n    return 0; \n}" > ./src/main.c
