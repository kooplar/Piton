#Bryan MArtin Tostado
#creation 12 june 2014
#Makefile for virtual machine for PL/0 code 

CC=gcc
CFLAGS=-c -std=c99 
LDFLAGS= -lm #math lib for pow()
SOURCES= $(wildcard *.c) 
OBJECTS= $(SOURCES:.c=.o)
EXECUTABLE= piton

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
	sudo cp $(EXECUTABLE) /usr/local/bin
	
.PHONY: uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(EXECUTABLE)

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
	@echo "    backup : created copies of the project to ./backup/"
	@echo "    install : installs to /usr/bin "
	@echo "    unistall"
