#Bryan MArtin Tostado
#creation 9 march 2014
#Makefile for compiler project Piton

CC=gcc
CFLAGS=-c -std=c99
LDFLAGS=
SOURCES= $(wildcard *.c) 
OBJECTS= $(SOURCES:.c=.o)
EXECUTABLE= pc

all: build

.PHONY: debug
debug: CFLAGS+= -g
debug: clean build

.PHONY: build
build: $(OBJECTS) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f *.o $(EXECUTABLE) 

.PHONY: install
install: $(EXECUTABLE)
	sudo cp $(EXECUTABLE) /usr/bin
	
.PHONY: uninstall
uninstall:
	sudo rm -f /usr/bin/$(EXECUTABLE)

.PHONY: backup
backup:
	@mkdir -p ./backup/include #make folder (don't complain if it already exists)
	@mkdir -p ./backup/src #make folder (don't complain if it already exists)
	@cp -u ./include/*.h ./backup/include &> /dev/null #move header file backups into desired folder
	@cp -u *.c ./backup/src &> /dev/null #move source file backups into desired folder

.PHONY: help
help:
	@echo " available commands:"
	@echo "    build : builds Piton"
	@echo "    debug : builds Piton with debug symbols"
	@echo "    clean "
	@echo "    backup "
	@echo "    install : installs to /usr/bin "
	@echo "    unistall"
