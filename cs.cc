// CS Class
//
// © 2010 David J. Goehrig
//

#include "cs.h"

ALLOC_METHODS(CS)

INIT_METHOD(CS)
	PROPERTY(name);
	PROPERTY(ipaddr);
	INT_PROPERTY(port);
	INT_PROPERTY(nat_port);
	return true;
END

TOS_METHOD(CS)
	return "type:" + type() 
		+ ",name:" + name 
		+ ",ipaddr:" + ipaddr 
		+ ",port:" + string_of_int(port) 
		+ ",nat_port:" + string_of_int(nat_port);
END

