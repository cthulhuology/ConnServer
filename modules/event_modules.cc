// event_modules.cc
//
// Copyright (C) 2005 David J. Goehrig
// All rights reserved
//
//	Modules for processing events

#include "event_modules.h"
#include "database.h"
#include "data.h"
#include "server.h"


BEGIN_REGISTRY
REGISTER(new-event,NewEventModule);
REGISTER(cancel-event,CancelEventModule);
REGISTER(pending,PendingEventsModule);
REGISTER(replay-list,ReplayListModule);
REGISTER(replay,ReplayModule);
REGISTER(next,NextModule);
END_REGISTRY


MODULE_PROCESS(NewEventModule)
{
	MSG_CHECK( "name,game,date,season,limit" )
//	MSG_CHECK( "name,game,date,season,win,limit" )
	LOGIN_CHECK

	Database* d = DBPool::grab();
	d->begin();
	list<string> event_ids;
	list<string> dates = split('!',m["date"]);
	list<string>::iterator i;
	list<Event*> events;
	Tournament* t = NULL;
	if (! m["rounds"].empty()) {

		MSG_CHECK( "cost,chips,rebuys.cost,rebuys.chips,rebuys.end_round,addon.cost,addon.chips,rebuys.threshold" )

		t = Tournament::alloc(d);	
		if (t == NULL) {
			d->rollback();
			m.add("status","1");
			return c->send(m);
		}
		t->current = 0;
		t->cost = int_of_string(m["cost"]);
		t->chips = int_of_string(m["chips"]);
		t->rebuys.cost = int_of_string(m["rebuys.cost"]);
		t->rebuys.chips = int_of_string(m["rebuys.chips"]);
		t->rebuys.end_round = int_of_string(m["rebuys.end_round"]);
		t->rebuys.threshold = int_of_string(m["rebuys.threshold"]);
		t->addon.cost = int_of_string(m["addon.cost"]);
		t->addon.chips = int_of_string(m["addon.chips"]);
		
		list<string> rnds = split(',',m["rounds"]);
		list<string>::iterator i;
		for ( i = rnds.begin(); i != rnds.end(); ++i) {
			struct round r;
			list<string> fields = split('!',*i);
			r.duration = int_of_string(fields.front());
			fields.pop_front();	
			r.small_blind = int_of_string(fields.front());
			fields.pop_front();	
			r.big_blind = int_of_string(fields.front());
			fields.pop_front();	
			r.ante = int_of_string(fields.front());
			fields.pop_front();	
			r.break_time = int_of_string(fields.front());
			fields.pop_front();	
			t->rounds.push_back(r);
		}
		if (! t->save(d)) {
			d->rollback();
			m.add("status","1");
			return c->send(m);	
		}
	}
	Season* season = Cache::find<Season>(m["season"]);
	if ( season == NULL) {
		d->rollback();
		m.add("status","1");
		DBPool::release(d);
		return c->send(m);
	}
	for (i = dates.begin(); i != dates.end(); ++i) {
		Event* event = Event::alloc(d);
		event->owner = c->login()->player;
		event->name = m["name"];
		event->notes = m["notes"];
		event->game = m["game"];
		event->playoff = (m["playoff"] == "1" ? true : false);
		event->date = int_of_string(*i);
		event->tournament = t;
		event->season = season;
		event->options = Options::alloc(d);
	//	event->options->win = static_cast<Options::WinType>(int_of_string(m["win"]));
		event->options->value = int_of_string(m["value"]);
		event->options->limit = static_cast<Options::LimitType>(int_of_string(m["limit"]));
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
		event->options->hilo = int_of_string(m["hilo"]);
		if (! event->save(d) || ! event->options->save(d)) {
			d->rollback();
			m.add("status","1");
			DBPool::release(d);
			return c->send(m);
		}
		event_ids.push_back(string_of_Uint64(event->id) + "!" + event->name + "!" + string_of_int(event->date));
		events.push_back(event);
	}
	if (m["playoff"] == "1") {
		season->addPlayoffs(events,m["qualifiers"]);
	} else {
		season->addEvents(events);
	}
	if (! season->save(d) || ! d->commit()) {
		m.add("status","1");	
		d->rollback();
		DBPool::release(d);
		return c->send(m);
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
	Season* s = Cache::find<Season>(m["season"]);
	
	s->events.remove(e);
	s->playoffs.remove(e);
	Server::removeEvent(e);

	Database* d = DBPool::grab();
	d->begin();
	if (! s->save(d) || ! d->commit()) {
		list<Event*> el;
		el.push_back(e);
		s->addEvents(el);
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

MODULE_PROCESS(ReplayListModule)
{
	MSG_CHECK("event");
	LOGIN_CHECK
	
	Event* e = Cache::find<Event>(m["event"]);
	
	if (e == NULL) {
		cerr << "[ReplayListModule] no such event " << m["event"] << endl;
		m.add("status","1");
		return c->send(m);
	}
	
	list<Table*>::iterator j;
	for (j = e->tables.begin(); j != e->tables.end(); ++j) {
		if (*j == NULL) continue;	
		list<Game*>::iterator i;
		int hand = 1;
		string replays;
		for (i = (*j)->games.begin(); i != (*j)->games.end(); ++i) {
			if (*i == NULL) continue;
			if ( ! (*i)->done  || (*i)->replay == NULL) continue;
			if (replays.empty()) {
				replays = string_of_Uint64((*i)->replay->id);
			} else {
				replays += "!" + string_of_Uint64((*i)->replay->id);
			}
		}
		m.add((*j)->name, replays);
	}
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(ReplayModule)
{
	//MSG_CHECK("replay");
	if ( m["replay"].empty() && m["game"].empty() ) {
		m.add("status","1");
		return c->send(m);
	}
	
	Replay* r;
	if (m["game"].empty()) {
		r = Cache::find<Replay>(m["replay"]);
	} else {
		Game* g = Cache::find<Game>(m["game"]);
		if (g == NULL) {
			m.add("status","1");
			return c->send(m);
		}
		r = g->replay;
	}
	
	if (r == NULL) {
		cerr << "[ReplayModule] no such replay " << m["replay"] << endl;
		m.add("status","1");
		return c->send(m);
	}

	if (c->login() == NULL) {
		Login* login = Login::alloc(NULL);
		login->player = NULL;
		login->server = Server::cs();
		login->sock = c;
		login->replay = r;
		login->frame = 0;
		c->set_login(login);
	} else {
		c->login()->replay = r;
		c->login()->frame = 0;
	}

	if( ! r->send(c,0)) {
		c->login()->replay = NULL;
		c->login()->frame = 0;
		m.add("status","1");
		return c->send(m);
	}
	return true;
}


MODULE_PROCESS(NextModule)
{
	if (c->login() == NULL) return false;

	Replay* r = c->login()->replay;
	int frame = ++c->login()->frame;
	if (r == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	r->send(c,frame);
	return true;
}
