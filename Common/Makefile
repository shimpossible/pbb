include ../include.mk

SHLIB      = $(CXX) -shared
SHLIBFLAGS =

objs = pbb

OBJ_DIR := $(ROOT_DIR)/objs/Common
SRC_DIR := src
INC_DIR := include
DEP_DIR := deps
LIB_DIR := $(ROOT_DIR)/lib

INCLUDE =-Iinclude

DEP = $(CXX) -M
MKDIR = mkdir -p

LIB_STATIC_DEBUG  = $(LIB_DIR)/pbb.CommonD.a
LIB_STATIC_RELEASE= $(LIB_DIR)/pbb.Common.a
LIB_SHARED_DEBUG  = $(LIB_DIR)/pbb.CommonD.so

all: $(LIB_STATIC_DEBUG) $(LIB_SHARED_DEBUG)

$(DEP_DIR)/%.d: $(SRC_DIR)/%.cpp
	@echo "Creating dependency list for " $^
	$(MKDIR) $(DEP_DIR)
	$(DEP) $(SRC_DIR)/$(patsubst %.d,%.cpp,$(notdir $@)) $(INCLUDE) $(CXXFLAGS) >$@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	echo "Compiling " $< "(debug, static)"
	$(MKDIR) $(OBJ_DIR)
	$(CXX) $(INCLUDE) $(CXXFLAGS) $(OPT_CXX) -c $< -o $@

$(LIB_STATIC_DEBUG): $(foreach o,$(objs),$(OBJ_DIR)/$(o).o)
	ar -cr $@ $^

$(LIB_SHARED_DEBUG): $(foreach o,$(objs),$(OBJ_DIR)/$(o).o)
	$(SHLIB) $(SHLIBFLAGS) -shared -o $@ $^
