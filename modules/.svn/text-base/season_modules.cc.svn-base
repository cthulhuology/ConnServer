// season_modules.cc
//
//	Copyright (C) 2005 David J. Georhig
//	All rights resered
//
//	modules for manipulating seasons

#include "season_modules.h"

#include "data.h"
#include "database.h"
#include "util.h"
#include "user_map.h"
#include "server.h"

BEGIN_REGISTRY
REGISTER(new-season,NewSeasonModule);
REGISTER(get-seasons,GetSeasonsModule);
REGISTER(my-seasons,MySeasonsModule);
REGISTER(get-events,GetEventsModule);
REGISTER(cancel-season,CancelSeasonModule);
REGISTER(my-events,MyEventsModule);
END_REGISTRY


MODULE_PROCESS(NewSeasonModule)
{
	LOGIN_CHECK
	MSG_CHECK("league,name")

	League* league = Cache::find<League>(m["league"]);
	if (league == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	
	Login* login = UserMap::getLogin(c);
	if (login->player != league->commish) {
		m.add("status","3");
		return c->send(m);
	}
	
	list<Season*>::iterator i;
	for (i = league->seasons.begin(); i != league->seasons.end(); ++i) {
		if (m["name"] == (*i)->name) {
			m.add("status","2");
			return c->send(m);
		}
	}

	Database* d = DBPool::grab();
	d->begin();
	Season* season = Season::alloc(d);
	season->name = m["name"];
	season->has_playoffs = m["playoffs"];
	season->ranking = m["ranking"];
	season->current = NULL;
	season->state = Season::PRESEASON;
	season->league = league;
	league->seasons.push_back(season);
	
	if (! season->save(d) || ! league->save(d)) {
		d->rollback();
		cerr << "[NewSeasonModule] Failed to save season" << endl;
		league->seasons.pop_back();
		m.add("status","1");
		DBPool::release(d);
		return c->send(m);
	}
	if (! d->commit()) {
		d->rollback();
		cerr << "[NewSeasonModule] Failed to commit" << endl;
		league->seasons.pop_back();
		m.add("status","1");
	} else {
		m.add("season",string_of_Uint64(season->id));
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}


MODULE_PROCESS(GetSeasonsModule)
{
	LOGIN_CHECK
	MSG_CHECK("league")

	League* league = Cache::find<League>(m["league"]);
	if (league == NULL) {
		m.add("status","1");
		return c->send(m);
	}
		
	list<Season*>::iterator i;
	for (i = league->seasons.begin(); i != league->seasons.end(); ++i) {
		m.add(string_of_Uint64((*i)->id),(*i)->name + "!" + string_of_int((*i)->state) + "!" + (*i)->has_playoffs + "!" + (*i)->ranking );
	}

	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(MySeasonsModule)
{
	LOGIN_CHECK
	Login* login = c->login();

	list<League*>::iterator i;
	list<Season*>::iterator j;
	for (i = login->player->leagues.begin(); i != login->player->leagues.end(); ++i) {
		for ( j = (*i)->seasons.begin(); j != (*i)->seasons.end(); ++j) {
			if (*j != NULL) {
				m.add(string_of_Uint64((*i)->id) + "!" + string_of_Uint64((*j)->id), (*i)->name + " - " + (*j)->name + "!" + string_of_int((*j)->state));
			}
		}
	}
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(GetEventsModule)
{
	LOGIN_CHECK
	MSG_CHECK("season")

	Season* season = Cache::find<Season>(m["season"]);
	if (season == NULL) {
		m.add("status","1");	
		return c->send(m);
	}
	list<Event*>::iterator i;
	for (i = season->events.begin(); i != season->events.end(); ++i) {
		if (*i != NULL ) {
			m.add(string_of_Uint64((*i)->id), (*i)->name + "!" + string_of_int((*i)->date) + "!0!" + ((*i)->tournament == NULL ? "0!0" : ((*i)->tournament->is_registered(c->login()->player) ? "1!1" : "1!0" )) + "!" +  (*i)->notes);
		}
	}
	m.add("qualifiers",season->qualifiers);
	for (i = season->playoffs.begin(); i != season->playoffs.end(); ++i) {
		if (*i != NULL ) {
			m.add(string_of_Uint64((*i)->id), (*i)->name + "!" + string_of_int((*i)->date) + "!1!" + ((*i)->tournament == NULL ? "0!0" :  ((*i)->tournament->is_registered(c->login()->player) ? "1!1" : "1!0" )) + "!" +  (*i)->notes);
		}
	}
	m.add("status","0");
	return c->send(m);
}


MODULE_PROCESS(CancelSeasonModule)
{
	LOGIN_CHECK
	MSG_CHECK("season")

	Season* season = Cache::find<Season>(m["season"]);
	if (season == NULL) {
		cerr << "[CancelSeasonModule] season not found " << m["season"] << endl;
		m.add("status","1");
		return c->send(m);
	}

	League* league = season->league;
	if (league == NULL) {
		cerr << "[CancelSeasonModule] league not found" << endl;
		m.add("status","1");
		return c->send(m);
	}

	list<Event*>::iterator i;
	for (i = season->events.begin(); i != season->events.end(); ++i) {
		if (*i == NULL) continue;	
		Server::removeEvent(*i);
	}
	for (i = season->playoffs.begin(); i != season->playoffs.end(); ++i) {
		if (*i == NULL) continue;	
		Server::removeEvent(*i);
	}
	Server::saveEvents();

	league->seasons.remove(season);
	
	Database* d = DBPool::grab();
	d->begin();

	if (! league->save(d) || ! d->commit()) {
		cerr << "[CancelSeasonModule] save failed" << endl;	
		league->seasons.push_back(season);
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(MyEventsModule)
{
	LOGIN_CHECK

	Server::myEvents(c->login()->player,m);
	m.add("status","0");
	return c->send(m);
}
