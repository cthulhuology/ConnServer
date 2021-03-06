// Player Class
//
// © 2010 David J. Goehrig

#include "player.h"
#include "details.h"

ALLOC_METHODS(Player)

INIT_METHOD(Player)
	PROPERTY(username);
	PROPERTY(password);
	PROPERTY(avatar);
	OBJ_PROPERTY(details,Details);
	return details != NULL;
END

TOS_METHOD(Player)
	return "type:" + type() 
		+ ",username:" + username 
		+ ",avatar:" + avatar 
		+ ",password:" + password 
		+ ",details:" + ( details == NULL ? "" : string_of_Uint64(details->id))
		;
END

