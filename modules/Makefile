
all : build

include ../.config

SRCS = $(wildcard *.cc)
MODS = $(patsubst %.cc,%_$(VERSION).dylib, $(wildcard *.cc))

include $(ARCH).mk


.PHONY: modules
modules: $(MODS)

.PHONY: clean
clean:
	rm -rf *.o *.dylib *.dSYM

.PHONY: build
build: 
	./build.sh build

.PHONY: install
install:
	./build.sh install

.PHONY: clean_install
clean_install: 
	sudo rm -rf /usr/local/lib/ConnServer/*.dylib
	$(MAKE) install

.PHONY: depends
depends:
	mkdep $(SRCS) $(CFLAGS)

.depends:
	touch .depends
	$(MAKE) depends

include .depend
# DO NOT DELETE
