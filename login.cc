// Login Class
//
// © 2010 David J. Goehrig
//

#include "login.h"

Login::Login() {}
Login::~Login() {}

Login* Login::alloc(Database* d)
{
	Login* retval = new Login();
	return retval;
}

INIT_METHOD(Login)
	OBJ_PROPERTY(player,Player);
	OBJ_PROPERTY(server,CS);
	sock = NULL;
	return player != NULL && sock != NULL;
END

TOS_METHOD(Login)
	return "type:" + type() 
		+ "\nplayer:" + (player == NULL ? "" : string_of_Uint64(player->id))
		+ "\nserver:" + (server == NULL ? "" : string_of_Uint64(server->id));
END

