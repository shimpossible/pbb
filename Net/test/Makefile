LIBRARY_NAME := Net
INCLUDE      := -I../include -I../../Thread/include -I../../Common/include

objs =
objs += SocketTest
objs += main
objs += EchoServer

TARGET=test

ifndef ROOT_DIR
        export ROOT_DIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/../..
endif

LINK       = $(CXX)
LINK_FLAGS = -L$(EXTERN_DIR)/gtest-1.7.0/lib/ -L$(LIB_DIR) -Wl,-Bstatic -lpbb.Net -lpbb.Thread -Wl,-Bdynamic -lpthread -lgtest
SHLIB      = $(CXX)
SHLIBFLAGS = -shared

OPT_CXX    =
OPT_SHARED_CXX = $(OPT_CXX) -fPIC
OPT_STATIC_CXX = $(OPT_CXX)

OBJ_SHARED_DIR := objs/shared
OBJ_STATIC_DIR := objs/static
SRC_DIR        := src
INC_DIR        := include
DEP_DIR        := deps
LIB_DIR        := $(ROOT_DIR)/lib
EXTERN_DIR     := $(ROOT_DIR)/extern
DEP            := $(CXX) -MM -MQ
MKDIR          := mkdir -p

LIB_STATIC_DEBUG    := $(LIB_DIR)/pbb.$(LIBRARY_NAME)D.a
LIB_SATIC_RELEASE   := $(LIB_DIR)/pbb.$(LIBRARY_NAME).a
LIB_SHARED_DEBUG    := $(LIB_DIR)/pbb.$(LIBRARY_NAME)D.so
LIB_SHARED_RELEASE  := $(LIB_DIR)/pbb.$(LIBRARY_NAME).so

DIRS = $(DEP_DIR) $(OBJ_SHARED_DIR) $(OBJ_STATIC_DIR) $(LIB_DIR)
all: $(DIRS) $(TARGET)

$(DIRS):
	@echo making $@
	$(MKDIR) $@

$(OBJ_SHARED_DIR)/%.o:  $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	@echo "Compiling " $< "(debug, shared)"
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(OPT_SHARED_CXX) -c $< -o $@

$(TARGET): $(foreach o,$(objs),$(OBJ_SHARED_DIR)/$(o).o)
	@echo "Linking" $(LINK_FLAGS)
	$(LINK) -o $@ $^  $(LINK_FLAGS)	

$(DEP_DIR)/%.d: $(DEP_DIR) $(SRC_DIR)/%.cpp
	@echo "Creating dependency list for " $^
	$(DEP) $(OBJ_STATIC_DIR)/$(patsubst %.d,%.o,$(notdir $@)) $(SRC_DIR)/$(patsubst %.d,%.cpp,$(notdir $@)) $(INCLUDE) $(CXXFLAGS) >$@


