// Login Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_LOGIN_H__
#define __HAVE_LOGIN_H__

#include "player.h"
#include "cs.h"
#include "socket.h"

#define LOGIN_CHECK if(!c->isBot() && (c->login() == NULL || c->login()->player == NULL)) return false;

#define BOT_CHECK if (! c->isBot()) return false;

class Login : public Object {
	public:
		Player* player;
		CS* server;	
		Socket* sock;	// DYNAMIC NOT SAVED

	COMMON_METHODS(Login);
};

#endif
