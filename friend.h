// Friend Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_FRIEND_H__
#define __HAVE_FRIEND_H__

#include "cache.h"
#include "player.h"

class Friend : public Object {
	public:
		Player* player;
		string category;
		string rating;
		string personality;
		string notes;	
	
	COMMON_METHODS(Friend)
};

#endif
