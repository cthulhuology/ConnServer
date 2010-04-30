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
REGISTER(start-pickup,StartPickupModule);
REGISTER(table-started,TableStartedModule);
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

	string tabs;
	
	int num_tables = event->create_tables();
	cerr << "[StartEventModule] allocating " << num_tables << " tables " << endl;
	list<Table*>::iterator i;
	for (i = event->tables.begin(); i != event->tables.end(); ++i) {
		if (*i == NULL) continue;
		if (tabs.empty()) {
			tabs = string_of_Uint64((*i)->id);	
		} else {
			tabs += "|" + string_of_Uint64((*i)->id);
		}
	}
	if (! d->commit()) {
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}
	if (event->tournament != NULL) {
		m.add("tournament",string_of_Uint64(event->tournament->id));
	}
	m.add("game", event->game);
	m.add("tables",tabs);		
	m.add("status","0");
	return c->send(m);	
}

MODULE_PROCESS(TableStartedModule)
{
	LOGIN_CHECK
	MSG_CHECK("event,table")

	Event* event = Cache::find<Event>(m["event"]);
	Table* table = Cache::find<Table>(m["table"]);

	event->tables.push_back(table);
	
	Database* d = DBPool::grab();
	d->begin();
	
	if (! event->save(d) || ! d->commit() ) {
		cerr << "[TableStartedModule] failed to save event state" << endl;
		d->rollback();
		m.add("status","1");	
	} else {
		m.add("status","0");	
	}
	DBPool::release(d);
	return c->send(m);	
}

MODULE_PROCESS(StartPickupModule)
{
	LOGIN_CHECK
	MSG_CHECK( "name,game,limit,stages" )
//	MSG_CHECK( "name,game,win,limit,stages" )

	Database* d = DBPool::grab();
	d->begin();

	Event* event = Event::alloc(d);
	event->owner = c->login()->player;
	event->name = m["name"];
	event->notes = m["notes"];
	event->game = m["game"];
	event->date = time(NULL);	
	event->playoff = false;
	event->tournament = NULL;
	event->season = NULL; // TODO ADD A SEASON FOR PICKUP GAMES
	event->options = Options::alloc(d);
//	event->options->win = static_cast<Options::WinType>(int_of_string(m["win"]));
	event->options->limit = static_cast<Options::LimitType>(int_of_string(m["limit"]));
	event->options->value = 0;
	event->options->stages = static_cast<Options::BetStages>(int_of_string(m["stages"]));
	switch(event->options->limit) {
		case Options::LIMIT:
			switch(event->options->stages) {
				case Options::TWO:
					event->options->bet[1].no = int_of_string(m["bet[1].no"]);
					event->options->bet[1].min = int_of_string(m["bet[1].min"]);
					event->options->bet[1].max = int_of_string(m["bet[1].max"]);
				case Options::ONE:
					event->options->bet[0].no = int_of_string(m["bet[0].no"]);
					event->options->bet[0].min = int_of_string(m["bet[0].min"]);
					event->options->bet[0].max = int_of_string(m["bet[0].max"]);
					break;
			}
		case Options::POTLIMIT:
		case Options::NOLIMIT:
			event->options->bet[0].no = int_of_string(m["bet[0].no"]);
			event->options->bet[0].min = int_of_string(m["bet[0].min"]);
			event->options->bet[0].max = 0;
			event->options->bet[1].no = 0;
			event->options->bet[1].min = 0;
			event->options->bet[1].max = 0;
			break;
	}
	event->options->buyin.min = int_of_string(m["buyin.min"]);
	event->options->buyin.max = int_of_string(m["buyin.max"]);
	event->options->buyin.no = int_of_string(m["buyin.no"]);
	event->options->ante = int_of_string(m["ante"]);
	event->options->blinds.big = int_of_string(m["blinds.big"]);
	event->options->blinds.small = int_of_string(m["blinds.small"]);
	event->options->wildcards = m["wildcards"];
	event->options->purchase = int_of_string(m["purchase"]);
	
	Table* tab = Table::alloc(d);
	for (int j = 0; j < Table::MAX_SEATS; ++j) {
		tab->seats[j].player = NULL;
		tab->seats[j].seat = -1;
		tab->seats[j].purse = 0;
	}
	tab->name = event->name + "!1";
	tab->event = event;
	tab->current = NULL;

	if (! event->save(d) 
	|| ! event->options->save(d) 
	|| ! tab->save(d) 
	|| ! d->commit()) {
		d->rollback();
		m.add("status","1");
		DBPool::release(d);
		return c->send(m);
	} 

	Message msg;
	msg.add("msg","start-event");
	msg.add("event",string_of_Uint64(event->id));
	msg.add("tournament","");
	msg.add("time",string_of_int(event->date));
	msg.add("game", event->game);
	msg.add("tables",string_of_Uint64(tab->id));		
	msg.add("status","0");
	if (! Server::send_watcher(msg)) {
		m.add("status","1");
	} else {
		m.add("status","0");
		m.add("event", string_of_Uint64(event->id));
	}
	DBPool::release(d);
	return c->send(m);
}
