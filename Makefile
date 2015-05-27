include include.mk

all: pbb-Common pbb-Thread


pbb-Common:
	$(MAKE) -C Common
pbb-Thread:
	$(MAKE) -C Thread
