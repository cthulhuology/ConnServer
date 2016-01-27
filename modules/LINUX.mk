
LIBEXT = so

MODS = $(patsubst %.cc,%_$(VERSION).so, $(wildcard *.cc))
CFLAGS =  -I../ -I/usr/include/postgresql -ggdb  -I/usr/include/python2.7 -D$(ARCH)
LDFLAGS = -L../ -lpq -lcs  -lpython2.7
 
%_$(VERSION).so : %.cc
	g++ -DVERSION=$(VERSION) -shared -fPIC -DPIC $(CFLAGS) -o $@ $< 
