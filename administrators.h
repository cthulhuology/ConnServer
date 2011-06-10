// Administrators Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_ADMINISTRATORS_H__
#define __HAVE_ADMINISTRATORS_H__

#include "player.h"

class Administrators : public Object {
	public:
		list<Player*> players;

	COMMON_METHODS(Administrators)
};

#endif
