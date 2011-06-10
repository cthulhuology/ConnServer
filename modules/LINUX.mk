
LIBEXT = so

MODS = $(patsubst %.cc,%_$(VERSION).so, $(wildcard *.cc))
CFLAGS =  -I../ -I/usr/include/postgresql -ggdb  -I/usr/include/python2.3 -D$(ARCH)
LDFLAGS = -L../ -lpq -lcs  -lpython2.3
 
%_$(VERSION).so : %.cc
	g++ -DVERSION=$(VERSION) -shared -fPIC -DPIC $(CFLAGS) -o $@ $< 
