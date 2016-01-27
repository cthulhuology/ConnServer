// Player Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_PLAYER_H__
#define __HAVE_PLAYER_H__

#include "object.h"
#include "cache.h"
#include <list>
#include <algorithm>
#include "details.h"

class Player : public Object {
	public:
		string username;
		string password;
		string avatar;
		Details* details;

	COMMON_METHODS(Player)
};

#endif
