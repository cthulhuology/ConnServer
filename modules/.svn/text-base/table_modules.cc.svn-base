// table_modules.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
//	Modules for configuring tables
//

#include "table_modules.h"
#include "data.h"
#include "database.h"
#include "util.h"
#include "user_map.h"
#include "server.h"

BEGIN_REGISTRY
REGISTER(gametypes,GameTypesModule)
REGISTER(event-standings,EventStandingsModule)
REGISTER(tables,TablesModule)
REGISTER(options,OptionsModule)
REGISTER(getAvatars,GetAvatarsModule)
REGISTER(my-tables,MyTablesModule) 
END_REGISTRY


MODULE_PROCESS(GameTypesModule) {
	LOGIN_CHECK
 	m.add("0","Dealer's Choice");
	m.add("1","Texas Hold 'Em");
	m.add("2","Pineapple");
	m.add("3","Kings and Little Ones");
	m.add("4","5 Card Stud");
	m.add("5","5 Card Draw");
	m.add("6","7 Card Stud");
	m.add("7","Anaconda");
	m.add("8","Follow The Queen");
	m.add("9","Black Mariah");
	m.add("10","Omaha");
	m.add("11","The Good, The Bad, and The Ugly");
	m.add("12","Underwater Baseball");
//	m.add("13","2 Card Guts");
//	m.add("14","3 Card Guts");
//	m.add("15","Indian Poker");
//	m.add("16","3-5-7");
//	m.add("17","7/27");
//	m.add("18","StopJack");
	m.add("status","0");
	return c->send(m);	
}

MODULE_PROCESS(EventStandingsModule) 
{
	LOGIN_CHECK
	MSG_CHECK("event")

	Event* event = Cache::find<Event>(m["event"]);
	if (event == NULL) {
		cerr << "[EventStandingsModule] event not found " << m["event"] << endl;
		m.add("status","1");
		return c->send(m);
	}

	League* league = event->season->league;
	list<Member*>::iterator i;
		
//	for (i = league->members.begin(); i != league->members.end(); ++i) {
//		if ((*i) != NULL && (*i)->table != NULL) {
//			m.add((*i)->player->username,(*i)->table->name + "!"
//				+ string_of_int((*i)->current.rank) + "!" 
//				+ string_of_int((*i)->current.purse));
//		}
//	}
	cerr << "EventStandingsModule -- TODO " << endl;


	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(TablesModule)
{
	LOGIN_CHECK
	MSG_CHECK("event")
	
	Event* event = Cache::find<Event>(m["event"]);
	if (event == NULL) {
		cerr << "[TablesModule] event not found" << endl;
		m.add("status","1");
		return c->send(m);
	}
	
	list<Table*>::iterator i;
	for (i = event->tables.begin(); i != event->tables.end(); ++i) {
		if (*i == NULL) continue;
		string id = string_of_Uint64((*i)->id);
		cerr << "Table Name " << (*i)->name << endl;
		Room* rm = Server::room(id);
		cerr << "Room " << rm << endl;
		if (rm == NULL) continue;
		if (rm->isEmpty()) {
			cerr << "Room is empty" << endl;
			continue;
		}
		cerr << "Adding room" << endl;
		m.add(id,(*i)->name);
	}
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(OptionsModule) 
{
	LOGIN_CHECK

	if (m["table"].empty() && m["event"].empty()) {
		cerr << "[OptionsModule] no table specified" << endl;
		m.add("status","1");
		return c->send(m);
	}

	Event* event;
	if (! m["event"].empty()) {
		event = Cache::find<Event>(m["event"]);
		
	} else {
		Table* table = Cache::find<Table>(m["table"]);
		if ( table == NULL || table->event == NULL || table->event->options == NULL) {
			cerr << "[OptionsModule] no such table " << m["table"] << endl;
			m.add("status","1");
			return c->send(m);
		}
		event = table->event;
	}

	if (event == NULL) {
		cerr << "[OptionsModule] No such event found" << endl;
		m.add("status","1");
		return c->send(m);
	}

	Options* options = event->options;
	Tournament* tourney = event->tournament;
	if (options == NULL) {
		cerr << "[OptionsModule] No options found" << endl;
		m.add("status","1");
		return c->send(m);
	}

//	m.add("win", string_of_int(options->win));
	m.add("value", string_of_int(options->value));
	m.add("limit", string_of_int(options->limit));
	m.add("stages", string_of_int(options->stages));	
	m.add("buyin.min", string_of_int(options->buyin.min));
	m.add("buyin.max", string_of_int(options->buyin.max));
	m.add("buyin.no", string_of_int(options->buyin.no));
	m.add("bet[0].min", string_of_int(options->bet[0].min));
	m.add("bet[0].max", string_of_int(options->bet[0].max));
	m.add("bet[0].no", string_of_int(options->bet[0].no));
	m.add("bet[1].min", string_of_int(options->bet[1].min));
	m.add("bet[1].max", string_of_int(options->bet[1].max));
	m.add("bet[1].no", string_of_int(options->bet[1].no));
	m.add("ante",string_of_int(options->ante));
	m.add("blinds.small",string_of_int(options->blinds.small));
	m.add("blinds.big",string_of_int(options->blinds.big));
	m.add("wildcards",options->wildcards);
	m.add("purchase",string_of_int(options->purchase));
	m.add("hilo",string_of_int(options->hilo));

	if (tourney != NULL) {
		m.add("tournament", string_of_Uint64(tourney->id));
		m.add("cost",string_of_int(tourney->cost));
		m.add("chips",string_of_int(tourney->cost));
		m.add("rebuys.cost",string_of_int(tourney->rebuys.cost));
		m.add("rebuys.chips",string_of_int(tourney->rebuys.chips));
		m.add("rebuys.end_round",string_of_int(tourney->rebuys.end_round));
		m.add("rebuys.threshold",string_of_int(tourney->rebuys.threshold));
		m.add("addon.cost",string_of_int(tourney->addon.cost));
		m.add("addon.chips",string_of_int(tourney->addon.chips));
		list<struct round>::iterator i;
		string rnds;
		for (i = tourney->rounds.begin(); i != tourney->rounds.end(); ++i) {
			if (rnds.empty()) {
				rnds = string_of_int(i->duration) 
				+ "!" + string_of_int(i->small_blind) 
				+ "!" + string_of_int(i->big_blind) 
				+ "!" + string_of_int(i->ante) 
				+ "!" + string_of_int(i->break_time);
			} else {
				rnds += "," + string_of_int(i->duration) 
				+ "!" + string_of_int(i->small_blind) 
				+ "!" + string_of_int(i->big_blind) 
				+ "!" + string_of_int(i->ante) 
				+ "!" + string_of_int(i->break_time);
			}
		}
		m.add("rounds",rnds);
	} else {
		m.add("tournament","");
	}

	m.add("game",event->game);
	m.add("owner",event->owner->username);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(GetAvatarsModule)
{
	LOGIN_CHECK

	if (c->room() == Server::room("lobby")) {
		m.add("status","1");
		return c->send(m);
	}
	
	c->room()->addAvatars(m);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(MyTablesModule)
{
	LOGIN_CHECK

	Player* p = c->login()->player;
	list<Table*> tabs = Server::get_tables();
	list<Table*>::iterator j;
	list<League*>::iterator i;

	for (j = tabs.begin(); j != tabs.end(); ++j) {
		if ((*j) == NULL || (*j)->event == NULL) continue;
		if ((*j)->event->season  == NULL) {
			m.add(string_of_Uint64((*j)->id), (*j)->name + "!" + (*j)->event->name + "!" + (*j)->event->game + "!" + string_of_int((*j)->event->date) + "!Pick Up (" + (*j)->event->owner->username + ")");
		} else {
			for (i = p->leagues.begin(); i != p->leagues.end(); ++i) {
				if ((*j)->event->season->league == *i) {
					m.add(string_of_Uint64((*j)->id), (*j)->name + "!" + (*j)->event->name + "!" + (*j)->event->game + "!" + string_of_int((*j)->event->date) + "!" + (*i)->name);
				}
			}
		}
	}
	m.add("status","0");
	return c->send(m);		
}
