// Administrators Class
//
// © 2010 David J. Goehrig
//

#include "administrators.h"

ALLOC_METHODS(Administrators)

INIT_METHOD(Administrators)
	LIST_PROPERTY(players,Player);
END

TOS_METHOD(Administrators)
	return "type:" + type() + ",players=" + encode<Player>(players);	
END

