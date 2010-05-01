// watcher_modules.cc
//
//	Copyright (C) 2005 David J. Goehrig
//	All rights reserved
//

#include "watcher_modules.h"
#include "server.h"
#include "room.h"
#include "database.h"
#include "util.h"
#include "user_map.h"

BEGIN_REGISTRY
REGISTER(register-watcher,RegisterWatcherModule)
REGISTER(start-event,StartEventModule);
END_REGISTRY

MODULE_PROCESS(RegisterWatcherModule)
{
	cerr << "[RegisterWatcherModule]" << endl;
	Room* r = NULL;
	while (r == NULL) {
		r = Server::room("watchtower");
		if ( r == NULL) {
			cerr << "[Creating watchtower]" << endl;
			Server::addRoom("watchtower");
		}
	}
	cerr << "[Adding bot to watchtower]" << endl;
	r->add_bot(c);
	Server::set_watcher(c);
	m.add("status","0");
	cerr << "[Sending message]" << m.dump() << endl;
	return c->send(m);
}

MODULE_PROCESS(StartEventModule)
{
	if (! c->isBot()) return false;

	Event* event = Cache::find<Event>(m["event"]);
	if ( event == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	
	Database* d = DBPool::grab();
	d->begin();

	if (! d->commit()) {
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}
	m.add("status","0");
	return c->send(m);	
}
