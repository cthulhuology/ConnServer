// Details Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_DETAILS_H__
#define __HAVE_DETAILS_H__

#include "cache.h"

class Details : public Object {
	public:
		string name;
		string email;
		string phone;
		string cell;
		string street;
		string apartment;
		string city;
		string state;
		string zip;
		string country;
		string dob;
		string sex;
		string occupation;
		string income;
		string spam;
	
	COMMON_METHODS(Details)
};

#endif
