// Details Class
//
// © 2010 David J. Goehrig
//

#include "details.h"

ALLOC_METHODS(Details)

INIT_METHOD(Details)
	PROPERTY(name);
	PROPERTY(email);
	PROPERTY(phone);
	PROPERTY(cell);
	PROPERTY(street);
	PROPERTY(apartment);
	PROPERTY(city);
	PROPERTY(state);
	PROPERTY(zip);
	PROPERTY(country);
	PROPERTY(dob);
	PROPERTY(sex);
	PROPERTY(occupation);
	PROPERTY(income);
	PROPERTY(spam);
	return true;
END

TOS_METHOD(Details)
	return "type:" + type() 
		+ "&name:" + name 
		+ "&email:" + email 
		+ "&phone:" + phone 
		+ "&cell:" + cell 
		+ ",street:" + street 
		+ ",apartment:" + apartment 
		+ ",city:" + city 
		+ ",state:" + state 
		+ ",zip:" + zip 
		+ ",country:" + country 
		+ ",dob:" + dob 
		+ ",sex:" + sex 
		+ ",occupation:" + occupation 
		+ ",income:" + income 
		+ ",spam:" + spam;
END

