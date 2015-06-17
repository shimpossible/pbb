#
#
#
include "common.mk"

BUILD_DIRS = $(DEP_DIR) $(OBJ_SHARED_DIR) $(OBJ_STATIC_DIR) $(LIB_DIR)
 
all: $(BUILD_DIRS) $(LIB_STATIC) $(LIB_SHARED)
	@echo done

$(BUILD_DIRS):
	$(MKDIR) $@

$(DEP_DIR)/%.d: $(DEP_DIR) $(SRC_DIR)/%.cpp
	@echo "Creating dependency list for " $^
	$(DEP) $(OBJ_STATIC_DIR)/$(patsubst %.d,%.o,$(notdir $@)) $(SRC_DIR)/$(patsubst %.d,%.cpp,$(notdir $@)) $(INCLUDE) $(CXXFLAGS) >$@

# .SECONDARY: 
$(OBJ_STATIC_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	@echo "Compiling " $< "(static)"
	@$(CXX) $(INCLUDE) $(CXXFLAGS) $(OPT_STATIC_CXX) -c $< -o $@

$(OBJ_SHARED_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP_DIR)/%.d
	@echo "Compiling " $< "(shared)"
	@$(CXX) $(INCLUDE) $(CXXFLAGS) $(OPT_SHARED_CXX) -c $< -o $@

$(LIB_STATIC): $(foreach o,$(objs),$(OBJ_STATIC_DIR)/$(o).o)
	@ar -cr $@ $^

$(LIB_SHARED): $(foreach o,$(objs),$(OBJ_SHARED_DIR)/$(o).o)
	@echo "Creating shared libary " $@
	@$(SHLIB) $(SHLIBFLAGS) -o $@ $^

# include depency list
include $(addprefix $(DEP_DIR)/,$(addsuffix .d,$(objs)))
