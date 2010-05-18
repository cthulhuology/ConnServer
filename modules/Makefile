
all : build

include ../.config

SRCS = $(wildcard *.cc)
MODS = $(patsubst %.cc,%_$(VERSION).$(LIBEXT), $(wildcard *.cc))

include $(ARCH).mk


.PHONY: modules
modules: $(MODS)

.PHONY: clean
clean:
	rm -rf *.o *.$(LIBEXT) *.dSYM

.PHONY: build
build: 
	./build.sh build

.PHONY: install
install:
	./build.sh install

.PHONY: clean_install
clean_install: 
	sudo rm -rf /usr/local/lib/ConnServer/*.$(LIBEXT)
	$(MAKE) install

.PHONY: depends
depends:
	mkdep $(SRCS) $(CFLAGS)

.depends:
	touch .depends
	$(MAKE) depends

include .depend
# DO NOT DELETE
