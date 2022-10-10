# settings here:

BUILD_MODE?=DEBUG
PLATFORM?=PLATFORM_DESKTOP

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    CC:=gcc
    CXX:=g++
	AR:=ar
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    CC:=emcc
    CXX:=em++
	AR:=emar
endif
CFLAGS?=-Wall -Wno-narrowing
CSTD:=-std=c++17
RELEASE_OPTIM?= -O2

BUILD_DIR?=build/make/$(PLATFORM)_$(BUILD_MODE)/

# you dont need to worry about this stuff:

ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

# get current dir
current_dir :=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_MODE_CFLAGS:=
ifeq ($(BUILD_MODE),DEBUG)
	BUILD_MODE_CFLAGS += -g
else
	BUILD_MODE_CFLAGS +=$(RELEASE_OPTIM)
endif

MAKE_CMD?=make

ifeq ($(BUILD_DIR),build/)
	BUILD_DIR:=$(current_dir)$(BUILD_DIR)
endif

CDEPFLAGS=-MMD -MF ${@:.o=.d}

# rules:

.PHONY:all clean

all:
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_DIR)CPP_Utils_depFile.dep

clean:
	$(MAKE_CMD) -C "$(RAYLIB_DIR)/src" clean
	rm -rf $(BUILD_DIR)