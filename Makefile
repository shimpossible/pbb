#include include.mk

LIB_NAMES := Common Thread Messaging

all: $(LIB_NAMES)

$(LIB_NAMES):
	make -C $@

.PHONY: $(LIB_NAMES)
