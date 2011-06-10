
LIB=libcs.so

CFLAGS =  -D$(ARCH) -I/usr/include/postgresql -ggdb  -I/usr/include/python2.3

LDFLAGS = -L. -lpq -lcs -lpython2.3

$(LIB) : $(OBJS)
	g++ -shared -Wl,-soname,libcs.so -o libcs.so $(OBJS)

