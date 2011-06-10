
LIB = libcs.dylib

CFLAGS = -D$(ARCH) -ggdb -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6  -I/opt/local/include/postgresql84

LDFLAGS = -L. -L/opt/local/lib/postgresql84 -lpq -lcs -lpython2.6

$(LIB) : $(OBJS)
	MACOSX_DEPLOYMENT_TARGET=10.6 g++ -dynamiclib $(OBJS) -o $(LIB) -L/opt/local/lib/postgresql84/ -lpq -lpython2.6 -undefined dynamic_lookup -L.

