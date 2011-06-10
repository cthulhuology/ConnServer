
all: dbtool

include ../.config
include $(ARCH).mk

dbtool: dbtool.cc
	g++ -o dbtool -I.. -L.. $(CFLAGS) $(LDFLAGS) dbtool.cc

.PHONY: clean
clean:
	rm -rf *.o dbtool *.dSYM *.log

.PHONY:
depends:
	mkdep dbtool.cc $(CFLAGS) -I.. -L..

.depend:
	touch .depend
	$(MAKE) depends

include .depend
