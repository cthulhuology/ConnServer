LIBEXT=dylib

CFLAGS =  -I../ -D$(ARCH)  -ggdb  -I/System/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6  -I/opt/local/include/postgresql84 

LDFLAGS = -L../ -L/opt/local/lib/postgresql84 -lpq -lcs -lpython2.6

%_$(VERSION).dylib : %.cc
	MACOSX_DEPLOYMENT_TARGET=10.6 g++ -dynamiclib $< -DVERSION=$(VERSION) -fPIC -DPIC $(CFLAGS) -o $@ $(LDFLAGS) -undefined dynamic_lookup

