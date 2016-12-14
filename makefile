TARGET := serial

.PHONY: all clean uninstall  test install

default:all
all: $(TARGET)

SRC_DIR := .
SOURCE := $(wildcard $(SRC_DIR)/*.c)


PROJ_LIBS += -lpthread

CROSS_COMPILE?=arm-linux-gnueabihf-
CC=$(CROSS_COMPILE)g++
CCC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)g++



RM ?= -rm
INC := -I./

OBJS := $(patsubst %.c,%.o,$(SOURCE))

vpath %.c $(SRC_DIR)

$(TARGET):$(OBJS)
	@$(CCC)   $^  $(INC) $(PROJ_LIBS) -o $@

%.o:%.c
	$(CCC)  $(INC)  -c $< -o $@

test:
	@echo $(SOURCE)
	@echo $(OBJS)

clean:
	$(RM) $(TARGET) $(OBJS)

install:
	sysv-rc-conf --list | grep sudo || sudo apt-get install sysv-rc-conf -y
	sudo cp $(TARGET) /usr/local/bin/
	sudo cp $(TARGET).sh /etc/init.d/
	sudo chmod +x /etc/init.d/$(TARGET).sh
	cd /etc/init.d; sudo sysv-rc-conf $(TARGET).sh on

uninstall:
	sudo /etc/init.d/$(TARGET).sh stop
	cd /etc/init.d; sudo sysv-rc-conf $(TARGET).sh off
	sudo rm /etc/init.d/$(TARGET).sh
	sudo rm /usr/local/bin/$(TARGET)