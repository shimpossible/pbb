#
#

INCLUDE_FLAGS := $(foreach I,$(INCLUDE), -I$(I))

include $(ROOT_DIR)/build/common.mk

BUILD_DIRS = $(DEP_DIR) $(OBJ_SHARED_DIR) $(OBJ_STATIC_DIR) $(LIB_DIR)
all: $(BUILD_DIRS) $(TARGET)

$(BUILD_DIRS):
	@echo making $@
	$(MKDIR) $@

$(OBJ_STATIC_DIR)/%.o:  $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	@echo "Compiling " $< "(debug, static)"
	$(CXX) $(INCLUDE_FLAGS) $(CXXFLAGS) $(OPT_STATIC_CXX) -c $< -o $@

$(TARGET): $(foreach o,$(objs),$(OBJ_STATIC_DIR)/$(o).o)
	@echo "Linking" $@
	$(LINK) -o $@ $^  $(LINKFLAGS)	

$(DEP_DIR)/%.d: $(DEP_DIR) $(SRC_DIR)/%.cpp
	@echo "Creating dependency list for " $^
	$(DEP) $(OBJ_STATIC_DIR)/$(patsubst %.d,%.o,$(notdir $@)) $(SRC_DIR)/$(patsubst %.d,%.cpp,$(notdir $@)) $(INCLUDE_FLAGS) $(CXXFLAGS) >$@.tmp
	sed 's/\($*\)\.[ :]*/\1.o $(subst /,\/,$@) : /g' < $@.tmp > $@
	rm $@.tmp

