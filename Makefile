# **********************
# FileName: Makefile
# **********************

SRC_DIR = ./
INC_DIR = ./
OBJ_DIR = ./object/
BIN_DIR := ./
INSTALL_DIR := /usr/local/bin/
VER := v0.3

CROSS_COMPILE ?=arm-linux-gnueabihf-
CC=$(CROSS_COMPILE)gcc
LINK=$(CROSS_COMPILE)gcc

CFLAGS= -g -Wall -std=gnu99
LDFLAGS = -lm -lpthread -lrt -ldl
INCLUDE= -I./


SOURCES = $(wildcard $(SRC_DIR)*.c)
OBJECTS = $(SOURCES:$(SRC_DIR).c = $(SRC_DIR).o)
TARGET= meter_$(VER)

.PHONY:all clean install uninstall
all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) $^ -o $@ $(LDFLAGS)
%.o:%.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(LDFLAGS)

install:
	@cp $(TARGET) $(INSTALL_DIR)$(TARGET)_$(VER)

uninstall:
	@rm -f $(TARGET) $(INSTALL_DIR)$(TARGET)_$(VER)
	
clean:
	@rm -rf $(TARGET) $(SRC_DIR)*.o


#--------------------------- END OF FILE ---------------------------#


