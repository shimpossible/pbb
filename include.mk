ifndef ROOT_DIR
	export ROOT_DIR=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
endif
