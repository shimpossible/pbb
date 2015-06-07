ifndef ROOT_DIR
	export ROOT_DIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
endif


SHLIB      = $(CXX)
SHLIBFLAGS = -shared

OPT_CXX    =
OPT_SHARED_CXX = $(OPT_CXX) -fPIC
OPT_STATIC_CXX = $(OPT_CXX)

OBJ_SHARED_DIR := $(ROOT_DIR)/objs/$(LIBRARY_NAME)/shared
OBJ_STATIC_DIR := $(ROOT_DIR)/objs/$(LIBRARY_NAME)/static
SRC_DIR        := src
INC_DIR        := include
DEP_DIR        := deps
LIB_DIR        := $(ROOT_DIR)/lib

DEP            := $(CXX) -MM -MQ
MKDIR          := mkdir -p

LIB_STATIC_DEBUG    := $(LIB_DIR)/pbb.$(LIBRARY_NAME)D.a
LIB_SATIC_RELEASE   := $(LIB_DIR)/pbb.$(LIBRARY_NAME).a
LIB_SHARED_DEBUG    := $(LIB_DIR)/pbb.$(LIBRARY_NAME)D.so
LIB_SHARED_RELEASE  := $(LIB_DIR)/pbb.$(LIBRARY_NAME).so

all: build_dirs $(LIB_STATIC_DEBUG) $(LIB_SHARED_DEBUG) $(LIB_STATIC_RELEASE) $(LIB_SHARED_RELEASE)
	@echo done

build_dirs: $(DEP_DIR) $(OBJ_SHARED_DIR) $(OBJ_STATIC_DIR) $(LIB_DIR)

$(DEP_DIR) $(OBJ_SHARED_DIR) $(OBJ_STATIC_DIR) $(LIB_DIR):
	$(MKDIR) $@

$(DEP_DIR)/%.d: $(DEP_DIR) $(SRC_DIR)/%.cpp
	@echo "Creating dependency list for " $^
	$(DEP) $(OBJ_STATIC_DIR)/$(patsubst %.d,%.o,$(notdir $@)) $(SRC_DIR)/$(patsubst %.d,%.cpp,$(notdir $@)) $(INCLUDE) $(CXXFLAGS) >$@

.SECONDARY: 
$(OBJ_STATIC_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	@echo "Compiling " $< "(debug, static)"
	@$(CXX) $(INCLUDE) $(CXXFLAGS) $(OPT_STATIC_CXX) -c $< -o $@


$(OBJ_SHARED_DIR)/%.o: $(SRC_DIR)/$.cpp $(DEP_DIR)/%.d
	@echo "Compiling " $< "(debug, shared)"
	@$(CXX) $(INCLUDE) $(CXXFLAGS) $(OPT_SHARED_CXX) -c $< -o $@

$(LIB_STATIC_DEBUG): $(foreach o,$(objs),$(OBJ_STATIC_DIR)/$(o).o)
	@ar -cr $@ $^

$(LIB_SHARED_DEBUG): $(foreach o,$(objs),$(OBJ_SHARED_DIR)/$(o).o)
	@echo "Creating shared libary " $@
	@$(SHLIB) $(SHLIBFLAGS) -o $@ $^

$(LIB_SHARED_RELEASE): $(foreach o,$(objs),$(OBJ_SHARED_DIR)/$(o).o)
	@$(SHLIB) $(SHLIBFLAGS) -o $@ $^

include $(addprefix $(DEP_DIR)/,$(addsuffix .d,$(objs)))
