#Bryan MArtin Tostado
#creation 9 march 2014
#Makefile for compiler project Piton

CC=gcc
CFLAGS=-c -std=c99
LDFLAGS=
SOURCES= $(wildcard *.c) 
OBJECTS= $(SOURCES:.c=.o)
EXECUTABLE= pitonc

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
	cp $(EXECUTABLE) /usr/local/bin
	
.PHONY: uninstall
uninstall:
	rm -f /usr/local/bin/$(EXECUTABLE)

.PHONY: backup
backup:
	@mkdir -p ./backup/include
	@mkdir -p ./backup/src
	@cp -u ./include/*.h ./backup/include &> /dev/null
	@cp -u *.c ./backup/src &> /dev/null

.PHONY: help
help:
	@echo " available commands:"
	@echo "    build : builds Piton"
	@echo "    debug : builds Piton with debug symbols"
	@echo "    clean "
	@echo "    backup "
	@echo "    install : installs to /usr/local/bin "
	@echo "    unistall"
