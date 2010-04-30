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
	Table* t = Cache::find<Table>(m["room"]);
	if ( t == NULL) {
		cerr << "[BotLoginModule] no such table " << m["room"] << endl;
	}
	cerr << "[BotLoginModule] Bot connected to room " <<  m["room"] << " on socket " << c << endl;
	r->set_table(t);
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

	list<Table*>::iterator i;
	int count = 0;

	Table* t = r->table();
	Event* e;
	if (t == NULL) goto closing;
	e = t->event;
	if (e == NULL) goto closing;

	for (i = e->tables.begin(); i != e->tables.end(); ++i) {
		if (*i == NULL) continue;
		Room* rm = Server::room(string_of_Uint64((*i)->id));
		if (rm == r) continue;
		if (rm != NULL) ++count;
	}
	if (count > 0)  {
		cerr << "[BotDisconnectModule] other tables exist for this event, not ending" << endl;
	} else {
		cerr << "[BotDisconnectModule] no other tables found, finalizing event" << endl;
		if (e->season) {
			cerr << "[BotDisconnectModule] ranking players for season event" << endl;
			Message msg;
			msg.add("season",string_of_Uint64(e->season->id));
			Module::call("rank-players",c,msg);
		} else {
			cerr << "[BotDisconnectModule] no ranking for pickup game" << endl;
		}
	}

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

