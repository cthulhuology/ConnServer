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
		+ "\nname:" + name 
		+ "\nemail:" + email 
		+ "\nphone:" + phone 
		+ "\ncell:" + cell 
		+ "\nstreet:" + street 
		+ "\napartment:" + apartment 
		+ "\ncity:" + city 
		+ "\nstate:" + state 
		+ "\nzip:" + zip 
		+ "\ncountry:" + country 
		+ "\ndob:" + dob 
		+ "\nsex:" + sex 
		+ "\noccupation:" + occupation 
		+ "\nincome:" + income 
		+ "\nspam:" + spam;
END

