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
		+ "\nname:" + name 
		+ "\nipaddr:" + ipaddr 
		+ "\nport:" + string_of_int(port) 
		+ "\nnat_port:" + string_of_int(nat_port);
END

