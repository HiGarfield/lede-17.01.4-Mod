RM ?= rm -f

# Git version retrieval
UDP2RAW_GIT_VER := $(shell git describe --always --dirty 2>/dev/null || echo "unknown")
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
    -Os \
    -flto \
    -s

# Target name
NAME := udp2raw

# Source files
COMMON := $(filter-out pcap_wrapper.cpp, $(wildcard *.cpp lib/*.cpp))
SOURCES := $(COMMON) $(wildcard lib/aes_faster_c/*.cpp)
AES_ACC_C_SOURCES := $(wildcard lib/aes_acc/aes*.c)
AES_ACC_ASM_SOURCES := $(wildcard lib/aes_acc/asm/*.S)

# Assembly targets
AES_ACC_TARGETS := $(basename $(notdir $(AES_ACC_ASM_SOURCES)))

# Libraries
LIBS := -isystem libev -pthread -lrt

# Compiler options
COMPILE_OPT := -I. $(LIBS) $(FLAGS) $(EXTRA_FLAGS)

# Phony targets
.PHONY: all linux $(AES_ACC_TARGETS) git_version clean
all: linux

# Build targets
linux: git_version
	$(CXX) $(SOURCES) $(COMPILE_OPT) -o $(NAME)

$(AES_ACC_TARGETS): git_version
	$(CXX) $(COMMON) $(AES_ACC_C_SOURCES) lib/aes_acc/asm/$@.S $(COMPILE_OPT) -o $(NAME)

# Git version header generation
git_version:
	-@$(RM) git_version.h
	@echo $(UDP2RAW_GIT_VER_CODE) > git_version.h

# Clean up
clean:
	-$(RM) $(NAME) git_version.h
