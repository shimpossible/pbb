INCLUDE      := ../include ../../Thread/include ../../Common/include ../../extern/gtest-1.7.0/include
# search directors for libraries
LIB_DIRS     = $(LIB_DIR)
#libraries
LIBS         = pthread
#static libs to be linked in
STATIC_LIBS  = $(EXTERN_DIR)/gtest-1.7.0/lib/.libs/libgtest.a 
STATIC_LIBS += $(LIB_DIR)/libpbb.ThreadD.a 

objs =
objs += main

TARGET=test

ROOT_DIR=../..

include ../../build/exec.mk

