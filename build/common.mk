#
#
#

ifndef ROOT_DIR
	export ROOT_DIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/..
endif

# BUILD_TYPE = <debug> | <release>
BUILD_TYPE := debug

# 
# Directory names  
#
BUILD_DIR  := $(ROOT_DIR)/build
EXTERN_DIR := $(ROOT_DIR)/extern

SRC_DIR        := src
INC_DIR        := include
DEP_DIR        := deps
OBJ_DIR        := objs
LIB_DIR        := $(ROOT_DIR)/lib
EXTERN_DIR     := $(ROOT_DIR)/extern

ifeq ($(BUILD_TYPE), debug)
  LIB_SHARED      := $(LIB_DIR)/libpbb.$(LIBRARY_NAME)D.so
  LIB_STATIC      := $(LIB_DIR)/libpbb.$(LIBRARY_NAME)D.a  
  OBJ_SHARED_DIR  := $(OBJ_DIR)/shared/debug
  OBJ_STATIC_DIR  := $(OBJ_DIR)/static/debug
else
  LIB_SHARED      := $(LIB_DIR)/libpbb.$(LIBRARY_NAME).so
  LIB_STATIC      := $(LIB_DIR)/libpbb.$(LIBRARY_NAME).a
  OBJ_SHARED_DIR  := $(OBJ_DIR)/shared/release
  OBJ_STATIC_DIR  := $(OBJ_DIR)/static/release
endif

#
# Tools used during build
#

# common C++ compiler options
OPT_CXX    =
# options only for a SHARED library build
OPT_SHARED_CXX = $(OPT_CXX) -fPIC
# options only for a STATIC library build
OPT_STATIC_CXX = $(OPT_CXX)

ifeq ($(BUILD_TYPE), debug)
  OPT_SHARED_CXX += $(DEBUG_FLAGS)
  OPT_STATIC_CXX += $(DEBUG_FLAGS)
else
  OPT_SHARED_CXX += $(RELEASE_FLAGS)
  OPT_STATIC_CXX += $(RELEASE_FLAGS)
endif

# how to make dependency files
DEP            := $(CXX) -MM -MQ

# command to make a directory and all its children
MKDIR          := mkdir -p

SHLIB       := $(CXX)
SHLIBFLAGS  := -shared

LINK        := $(CXX)
LINKFLAGS   := -L$(EXTERN_DIR)/gtest-1.7.0/lib -L$(LIB_DIR) -W1,-Bstatic -lpthread -lgtest -lpbb.Net -lpbb.Thread
 
