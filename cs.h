// CS Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_CS_H__
#define __HAVE_CS_H__

#include "cache.h"

class CS : public Object {
	public:
		string name;
		string ipaddr;
		int port;
		int nat_port;

	COMMON_METHODS(CS);
};

#endif
