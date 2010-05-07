// admin.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
//	Administration modules for fun and profit
//

#include "admin.h"
#include "object.h"
#include "server.h"


BEGIN_REGISTRY
REGISTER(admin,AdminModule)
END_REGISTRY

MODULE_PROCESS(AdminModule) {
	Login* login = c->login();
	if (login == NULL || login->player == NULL) return false;
	CS* cs = Server::cs();
	if (cs == NULL || cs->admins == NULL) return false;
	list<Player*> admins = cs->admins->players;
	if (admins.empty()) return false;
	list<Player*>::iterator i;
	bool isAdmin = false;
	for (i = admins.begin(); i != admins.end(); ++i) {
		if (login->player == *i) {
			isAdmin = true;
			break;
		}	
	}
	if (! isAdmin) {
		return false;
	}
	if (! m["reload"].empty()) {
		Registry::load();
		return true;
	}
	
	if (! m["shutdown"].empty()) {
		exit(0);
	}
}



