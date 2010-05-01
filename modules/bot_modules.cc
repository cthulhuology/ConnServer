// bot_modules.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
//	Modules for Bots by Bots
//

#include "bot_modules.h"
#include "server.h"
#include "user_map.h"
#include "data.h"

BEGIN_REGISTRY
REGISTER(bot-login,BotLoginModule)
REGISTER(bot-send,BotSendModule)
REGISTER(bot-disconnect,BotDisconnectModule)
END_REGISTRY

MODULE_PROCESS(BotLoginModule) { 
	Room* r = NULL;
	while ( r == NULL) {
		r = Server::room(m["room"]);
		if (r == NULL) {
			Server::addRoom(m["room"]);
		}
	}
	r->add_bot(c);
	cerr << "[BotLoginModule] Bot connected to room " <<  m["room"] << " on socket " << c << endl;
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(BotSendModule) { 
	LOGIN_CHECK
	m.remove("msg");
	m.add("msg",m["bot-msg"]);
	m.remove("bot-msg");		
	if (c->room() == Server::room("lobby")) return false;
	c->room()->send_bot(m);
	return true;
}

MODULE_PROCESS(BotDisconnectModule) {
	cerr << "BotDisconnectModule" << endl;
	if (! c->isBot()) return false;
	Room* r = c->room();
closing:
	cerr << "[BotDisconnectModule] Disconnecting bot for room " << r->name << " on socket " << c << endl;
	Message msg;
	msg.add("msg","closing");
	msg.add("room",r->name);
	msg.add("status","0");
	cerr << "Broadcasting Message:" << endl;
	cerr << msg.dump() <<  endl;
	r->broadcast(msg);
	cerr << "Disconnecting members" << endl;
	r->disconnect_members();
	cerr << "Removing bot" << endl;
	r->remove_bot(c);
	cerr << "Diconnected bot" << endl;
	return true;
}

