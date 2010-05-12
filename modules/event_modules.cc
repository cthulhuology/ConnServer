// event_modules.cc
//
// Copyright (C) 2005 David J. Goehrig
// All rights reserved
//
//	Modules for processing events

#include "event_modules.h"
#include "database.h"
#include "object.h"
#include "server.h"
#include "login.h"

BEGIN_REGISTRY
REGISTER(new-event,NewEventModule);
REGISTER(cancel-event,CancelEventModule);
REGISTER(pending,PendingEventsModule);
END_REGISTRY


MODULE_PROCESS(NewEventModule)
{
	MSG_CHECK( "name,params,date" )
	LOGIN_CHECK

	Database* d = DBPool::grab();
	d->begin();
	list<string> event_ids;
	list<string> dates = split('|',m["date"]);
	list<string>::iterator i;
	list<Event*> events;
	for (i = dates.begin(); i != dates.end(); ++i) {
		Event* event = Event::alloc(d);
		event->name = m["name"];
		event->params = m["params"];
		event->date = int_of_string(*i);
		if (! event->save(d) ) {
			d->rollback();
			m.add("status","1");
			DBPool::release(d);
			return c->send(m);
		}
		event_ids.push_back(string_of_Uint64(event->id) + "|" + event->name + "|" + string_of_int(event->date));
		events.push_back(event);
	}
	Server::addEvents(events);
	Server::saveEvents();
	m.add("events",join(',',event_ids));
	m.add("status","0");
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(CancelEventModule)
{
	MSG_CHECK( "event,season" )
	LOGIN_CHECK

	Event* e = Cache::find<Event>(m["event"]);	
	Server::removeEvent(e);

	Database* d = DBPool::grab();
	d->begin();
	if (! d->commit()) {
		list<Event*> el;
		el.push_back(e);
		Server::addEvents(el);
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	Server::saveEvents();
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(PendingEventsModule)
{
	LOGIN_CHECK
	if (! Server::pendingEvents(m)) {
		cerr << "[PendingEventsModule] no events found" << endl;
	}
	m.add("status","0");
	return c->send(m);
}
