// Event Class
//
// © 2010 David J. Goehrig
//

#include "event.h"

ALLOC_METHODS(Event)

INIT_METHOD(Event)
	PROPERTY(name);
	PROPERTY(params);
	INT_PROPERTY(date);
	return true;
END

TOS_METHOD(Event)
	return "type:" + type() 
		+ ",name:" + name 
		+ ",params:" + params 
		+ ",date:" + string_of_int(date); 
END

