// chat_modules.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
#include "chat_modules.h"
#include "database.h"
#include "user_map.h"
#include "server.h"
#include "login.h"

// Chat Module
// 	for chat messages

BEGIN_REGISTRY
REGISTER(chat,ChatModule)
REGISTER(alert,AlertModule)
REGISTER(online,GetOnlineModule)
REGISTER(who,WhoModule)
END_REGISTRY

// Chat Module
//
// the "to" field is checked, if it is a room, then the Room::broadcast
// method is called, otherwise it is Server::imessaged to the specific
// user based on the uid

MODULE_PROCESS(ChatModule)
{
	LOGIN_CHECK
	if (! m["to"].empty()){
		if (m["to"] == "lobby") return false;	// YOU CAN'T SPAM THE LOBBY
		Room* r = Server::room(m["to"]);
		if (r == NULL) {
			if (Server::imessage(m)) {
				m.add("status","0");	
			} else {
				m.add("status","1");
			}
		} else {
			if (r->broadcast(m)) {
				m.add("status","0");
			} else {
				m.add("status","1");
			}
		}
		return c->send(m);
	} 
	return false;
}

// Alert Module
//	for admin messages

MODULE_PROCESS(AlertModule)
{
	LOGIN_CHECK
	Server::broadcast(m);
	return true;
}

MODULE_PROCESS(GetOnlineModule)
{
	LOGIN_CHECK
	string o = UserMap::users();

	cerr << "GetOnline " << o << endl;

	if (o.empty()) {
		m.add("status","1");
	} else {
		m.add("status","0");
		m.add("uids",o);
	}

	return c->send(m);
}


// WhoModule

MODULE_PROCESS(WhoModule)
{
	LOGIN_CHECK
	Room* r = c->room();
	if (r == Server::room("lobby")) {
		m.add("status","1");
	} else {
		if (c->isBot()) {
			r->bot_members(m);
		} else {
			r->members(m);
		}
		m.add("status","0");
	}
	return c->send(m);
}
