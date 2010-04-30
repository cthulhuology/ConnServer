include .config

all:
	$(MAKE) -f Makefile.$(ARCH)

clean: 
	$(MAKE) -f Makefile.$(ARCH) clean

install:
	$(MAKE) -f Makefile.$(ARCH) install

depends:
	$(MAKE) -f Makefile.$(ARCH) depends
# DO NOT DELETE
