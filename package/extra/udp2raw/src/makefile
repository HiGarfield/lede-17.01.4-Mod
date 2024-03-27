UDP2RAW_GIT_VER?=$(shell git rev-parse HEAD 2>/dev/null)
ifeq ($(UDP2RAW_GIT_VER),)
  UDP2RAW_GIT_VER:=unknown
endif

FLAGS:=-std=c++11 -Wall -Wextra \
	-Wno-unused-variable -Wno-unused-parameter -Wno-missing-field-initializers
EXTRA_FLAGS:=-ffunction-sections -fdata-sections \
	-Wl,--gc-sections,--as-needed \
	-DNDEBUG -fno-exceptions -fno-rtti -Os -s -flto

NAME:=udp2raw

COMMON:=$(filter-out pcap_wrapper.cpp, $(wildcard *.cpp lib/*.cpp))
LIBS:=-lpthread -isystem libev -lrt

SOURCES:=$(COMMON) $(wildcard lib/aes_faster_c/*.cpp)
SOURCES_AES_ACC=$(COMMON) $(wildcard lib/aes_acc/aes*.c) \
				lib/aes_acc/asm/$@.S
COMPILE_OPT:=-I. $(LIBS) $(FLAGS) $(EXTRA_FLAGS) -o $(NAME)
AES_ACC_TARGETS:=$(basename $(notdir $(wildcard lib/aes_acc/asm/*.S)))

.PHONY: linux $(AES_ACC_TARGETS) git_version clean
.DEFAULT_GOAL:= linux

linux: git_version
	$(CXX) $(SOURCES) $(COMPILE_OPT)

$(AES_ACC_TARGETS): git_version
	$(CXX) $(SOURCES_AES_ACC) $(COMPILE_OPT)

git_version:
	@echo "const char *gitversion = \"$(UDP2RAW_GIT_VER)\";" >git_version.h

clean:
	-rm -f $(NAME) git_version.h
