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
	INT_PROPERTY(time);
	return true;
END

TOS_METHOD(Event)
	return "type:" + type() 
		+ "\nname:" + name 
		+ "\nparams:" + params 
		+ "\ndate:" + string_of_int(date)
		+ "\ntime:" + string_of_int(time); 
END

