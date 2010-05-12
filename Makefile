
all : ConnServer dbtool modules

include .config

SRCS = administrators.cc cache.cc cs.cc database.cc details.cc dir.cc env.cc event.cc friend.cc interp.cc login.cc \
	message.cc module.cc object.cc player.cc pool.cc post.cc room.cc s2s.cc server.cc socket.cc user_map.cc util.cc

OBJS = $(patsubst %.cc,%.o,$(SRCS))

include $(ARCH).mk

%.o : %.cc
	g++ -fPIC -DPIC -c $< $(CFLAGS)

ConnServer : $(LIB) main.cc
	g++ -o ConnServer main.cc $(CFLAGS) $(LDFLAGS) 

.PHONY: modules
modules: 
	cd modules && $(MAKE)

.PHONY: modules_install
modules_install: modules
	cd modules && $(MAKE) clean_install

dbtool : $(LIB) dbtool/dbtool.cc
	cd dbtool && $(MAKE) dbtool

.PHONY: clean
clean:
	rm -rf *.o $(LIB) ConnServer *.dSYM *.log
	cd modules && $(MAKE) clean
	cd dbtool && $(MAKE) clean

.PHONY: install
install: ConnServer $(LIB)
	sudo cp $(LIB) /usr/local/lib
	echo "Running /sbin/ldconfig"
	sudo /sbin/ldconfig
	sudo cp CS ConnServer /usr/local/bin

depends:
	mkdep  $(SRCS) main.cc $(CFLAGS)
	cd modules && $(MAKE) depends
	cd dbtool && $(MAKE) depends

include .depend
# DO NOT DELETE
