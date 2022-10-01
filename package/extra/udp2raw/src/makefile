ifeq ($(OS),Windows_NT)
  DETECTED_OS:= Windows
else
  DETECTED_OS:= $(shell uname -s)
endif

ifeq ($(DETECTED_OS),Windows)
  UDP2RAW_GIT_VER?=$(shell git rev-parse HEAD 2>nul)
else
  UDP2RAW_GIT_VER?=$(shell git rev-parse HEAD 2>/dev/null)
endif

ifeq ($(UDP2RAW_GIT_VER),)
  UDP2RAW_GIT_VER="unknown"
endif

FLAGS:= \
	-std=c++11 -Wall -Wextra \
	-Wno-unused-variable \
	-Wno-unused-parameter \
	-Wno-missing-field-initializers
EXTRA_FLAGS:= \
	-Os -s -flto
ifeq ($(DETECTED_OS),Windows)
  EXTRA_FLAGS+= -static
endif

NAME:=udp2raw

ifeq ($(DETECTED_OS),Windows)
  PCAP:= pcap_wrapper.cpp
else
  PCAP:= -lpcap
endif
MP:=-DUDP2RAW_MP
COMMON:=$(filter-out pcap_wrapper.cpp, $(wildcard *.cpp lib/*.cpp))
LIBS:= -lpthread -isystem libev
ifeq ($(DETECTED_OS),Windows)
  LIBS+= -lws2_32
else
  LIBS+= -lrt
endif
SOURCES:=$(COMMON) $(wildcard lib/aes_faster_c/*.cpp)
SOURCES_AES_ACC=$(COMMON) $(wildcard lib/aes_acc/aes*.c) lib/aes_acc/asm/$@.S
COMPILE_OPT:= -I. $(LIBS) $(FLAGS) $(EXTRA_FLAGS) -o $(NAME)
AES_ACC_TARGETS:=$(basename $(notdir $(wildcard lib/aes_acc/asm/*.S)))

.PHONY: $(NAME) linux $(AES_ACC_TARGETS) pcap git_version clean

ifeq ($(DETECTED_OS),Linux)
$(NAME): linux
else
$(NAME): pcap
endif

linux: git_version
	$(CXX) $(SOURCES) $(COMPILE_OPT)

$(AES_ACC_TARGETS): git_version
	$(CXX) $(SOURCES_AES_ACC) $(COMPILE_OPT)

pcap: git_version
	$(CXX) $(SOURCES) $(PCAP) $(MP) $(COMPILE_OPT)

git_version:
ifeq ($(DETECTED_OS),Windows)
	echo const char *gitversion = "$(UDP2RAW_GIT_VER)"; >git_version.h
else
	echo "const char *gitversion = \"$(UDP2RAW_GIT_VER)\";" >git_version.h
endif

clean:
ifeq ($(DETECTED_OS),Windows)
	-del /f /q $(NAME).exe git_version.h
else
	-rm -f $(NAME) git_version.h
endif
