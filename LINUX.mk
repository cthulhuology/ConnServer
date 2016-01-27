
LIB=libcs.so

CFLAGS = -std=c++98 -D$(ARCH) -I/usr/include/postgresql -ggdb  -I/usr/include/python2.7

LDFLAGS = -L. -lpq -lcs -lpython2.7

$(LIB) : $(OBJS)
	g++ -shared -Wl,-soname,libcs.so -o libcs.so $(OBJS)

