// Event Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_EVENT_H__
#define __HAVE_EVENT_H__

#include "cache.h"

class Event : public Object {
	public:
		string name;
		string params;
		int date;
		time_t time;

	COMMON_METHODS(Event)
};

#endif
