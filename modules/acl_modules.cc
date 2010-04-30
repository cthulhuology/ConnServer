// acl_modules.cc
//
// Copyright (C) 2005 David J. Goehrig
// All rights reserved
//
//	Access Control List modules
//

#include "acl_modules.h"
#include "user_map.h"
#include "util.h"
#include "data.h"

BEGIN_REGISTRY
REGISTER(setAccess,SetAccessModule)
REGISTER(getAccess,GetAccessModule)
END_REGISTRY

MODULE_PROCESS(GetAccessModule)
{
	if (m["lid"].empty()) {
		m.add("status","1");
		return c->send(m);
	}

	Login* login;
	if (c->isBot()) {
		if (m["uid"].empty()) {
			m.add("status","1");
			return c->send(m);
		}
		login = UserMap::getLogin(m["uid"]);
	} else {
		login = UserMap::getLogin(c);
	}
	if (login == NULL || login->player == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	League* league = Cache::find<League>(m["lid"]);
	string ac = ""; 
	list<Member*>::iterator i;
	for (i = league->members.begin(); i != league->members.end(); ++i) {
		if ((*i)->player == login->player) {
			ac = (*i)->access;
		}
	}

	m.add("access",ac);	
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(SetAccessModule)
{
	if (m["uid"].empty()
	|| m["access"].empty()) {
		m.add("status","1");
		return c->send(m);
	}
	if (c->isBot()) {

	} else {

	}	
}
