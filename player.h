// Player Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_PLAYER_H__
#define __HAVE_PLAYER_H__

#include "cache.h"

class Friend;
class Post;
class Details;

class Player : public Object {
	public:
		string username;
		string password;
		string avatar;
		Details* details;
		list<Friend*> friends;
		list<Post*> posts;

	COMMON_METHODS(Player)
};

#endif
