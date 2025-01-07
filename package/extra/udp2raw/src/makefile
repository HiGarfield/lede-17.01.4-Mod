# Git version retrieval
UDP2RAW_GIT_VER ?= $(or $(shell git rev-parse HEAD 2>/dev/null),unknown)
UDP2RAW_GIT_VER_CODE := "const char *gitversion = \"$(UDP2RAW_GIT_VER)\";"

# Compiler flags
FLAGS := \
    -std=c++11 \
    -Wall \
    -Wextra \
    -Wno-unused-variable \
    -Wno-unused-parameter \
    -Wno-missing-field-initializers

EXTRA_FLAGS := \
    -ffunction-sections \
    -fdata-sections \
    -Wl,--gc-sections,--as-needed \
    -DNDEBUG \
    -fno-exceptions \
    -fno-rtti \
    -Os \
    -flto \
    -s

# Target name
NAME := udp2raw

# Source files
COMMON := $(filter-out pcap_wrapper.cpp, $(wildcard *.cpp lib/*.cpp))
SOURCES := $(COMMON) $(wildcard lib/aes_faster_c/*.cpp)
SOURCES_AES_ACC = $(COMMON) $(wildcard lib/aes_acc/aes*.c) \
                  lib/aes_acc/asm/$@.S

# Libraries
LIBS := -lpthread -isystem libev -lrt

# Compiler options
COMPILE_OPT := -I. $(LIBS) $(FLAGS) $(EXTRA_FLAGS) -o $(NAME)

# Assembly targets
AES_ACC_TARGETS := $(basename $(notdir $(wildcard lib/aes_acc/asm/*.S)))

# Phony targets
.PHONY: linux $(AES_ACC_TARGETS) git_version clean
.DEFAULT_GOAL := linux

# Build targets
linux: git_version
	$(CXX) $(SOURCES) $(COMPILE_OPT)

$(AES_ACC_TARGETS): git_version
	$(CXX) $(SOURCES_AES_ACC) $(COMPILE_OPT)

# Git version header generation
git_version:
	@$(RM) git_version.h
	@echo $(UDP2RAW_GIT_VER_CODE) > git_version.h

# Clean up
clean:
	@echo "Cleaning up..."
	-$(RM) $(NAME) git_version.h
