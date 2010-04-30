// tournament_modules.cc
//
//	Copyright (C) 2005 David J. Goehrig
//	All rights reserved
//
//	Tournament messages
//

#include <list>
#include <iostream>

using namespace std;

#include "tournament_modules.h"
#include "server.h"
#include "data.h"
#include "util.h"
#include "user_map.h"

BEGIN_REGISTRY
REGISTER(tournament-signup,TournamentSignUpModule)
REGISTER(tournament-withdraw,TournamentWithdrawModule)
REGISTER(tournament-seating,TournamentSeatingModule)
REGISTER(tournament-position,TournamentPositionModule)
REGISTER(seat-tournament,SeatTournamentModule)
REGISTER(reseat-tournament,ReseatTournamentModule)
REGISTER(register-tournament,RegisterTournamentModule)
REGISTER(start-round,StartRoundModule)
REGISTER(last-hand,LastHandModule)
REGISTER(start-break,StartBreakModule)
REGISTER(table-seating, TableSeatingModule)
REGISTER(pad-rounds,PadRoundsModule)
REGISTER(tournament-timer,TournamentTimerModule)
END_REGISTRY

MODULE_PROCESS(TournamentSignUpModule)
{
	LOGIN_CHECK
	MSG_CHECK("event")

	Player* p = c->login()->player;
	Event* e = Cache::find<Event>(m["event"]);
	if (p == NULL || e == NULL || e->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	if ( ! e->tournament->add_player(p)) {
		m.add("status","1");
		return c->send(m);
	}
	
	Database* d = DBPool::grab();
	d->begin();
	if ( ! e->tournament->save(d) || ! d->commit()) {
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(TournamentWithdrawModule)
{
	LOGIN_CHECK
	MSG_CHECK("event")

	Player* p = c->login()->player;
	Event* e = Cache::find<Event>(m["event"]);
	if (p == NULL || e == NULL || e->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	
	if ( ! e->tournament->remove_player(p)) {
		m.add("status","1");
		return c->send(m);
	}
	
	Database* d = DBPool::grab();
	d->begin();
	if ( ! e->tournament->save(d) || ! d->commit()) {
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}


MODULE_PROCESS(TournamentSeatingModule)
{
	LOGIN_CHECK
	MSG_CHECK("event")

	Event* e = Cache::find<Event>(m["event"]);
	if (e == NULL || e->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	
	m.add("seating",e->tournament->get_seating());

	return c->send(m);
}

MODULE_PROCESS(TournamentPositionModule)
{
	BOT_CHECK
	MSG_CHECK("pids")
	
	Table* t =  c->room()->table();
	if ( t == NULL || t->event == NULL || t->event->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	list<string> pids = split('!', m["pids"]);
	list<string>::iterator i;

	int dropouts = pids.size();
	for (i = pids.begin(); i != pids.end(); ++i) {
		Player* p = Cache::find<Player>(*i);
		if (p == NULL) continue;
		list<tournament_seating>::iterator j;
		for (j = t->event->tournament->seating.begin(); j != t->event->tournament->seating.end(); ++j) {
			if (p == j->player) {
				j->seat = -1;
				j->purse = 0;
				j->position = 1 + t->event->tournament->seating.size() - dropouts - t->event->tournament->current;	
			}
		}
	}
	t->event->tournament->current += dropouts;

	Database* d = DBPool::grab();
	d->begin();
	if (! t->event->tournament->save(d) || ! d->commit()) {
		d->rollback();
		cerr << "[TournamentPositionModule] failed to save positions" << endl;
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(SeatTournamentModule)
{
	BOT_CHECK
	MSG_CHECK("event")
	
	Event* e = Cache::find<Event>(m["event"]);
	if (e == NULL || e->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}	

	if (e->tournament->seating.size() <= 0) {
		cerr << "[SeatTournamentModule] failed no one to seat" << endl;
		m.add("status","1");
		return c->send(m);
	}
	e->tournament->seat_players(e->tables);

	Database* d = DBPool::grab();
	d->begin();
	if (! e->tournament->save(d) || ! d->commit()) {
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
	}		
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(ReseatTournamentModule) 
{
	BOT_CHECK
	
	Table* t = c->room()->table();
	if (t == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	Event* e = t->event;
	if (e == NULL || e->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}	
	
	e->tournament->seat_players(e->tables);

	Database* d = DBPool::grab();
	d->begin();
	if (! e->tournament->save(d) || ! d->commit()) {
		d->rollback();
		m.add("status","1");
		return c->send(m);
	}

	Message msg;
	msg.add("msg","reset-tournament");

	Room* rm = Server::room(string_of_Uint64(e->id));	
	if (rm == NULL) {
		cerr << "[ReseatTournamentModule] failed to find room " << e->id << endl;
		m.add("status","1");
		return c->send(m);
	} 

	rm->broadcast(msg);		

	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(RegisterTournamentModule)
{
//	LOGIN_CHECK
	MSG_CHECK("event")
	
	Room* rm = Server::addRoom(m["event"]);
	if (rm == NULL) {
		cerr << "[RegisterTournamentModule] failed to create a tourney room" << endl;
		m.add("status","1");
		return c->send(m);
	}

	rm->add_bot(c);
	rm->set_table(NULL);
	m.add("status","0");	
	return c->send(m);
}

MODULE_PROCESS(StartRoundModule)
{
	LOGIN_CHECK
	
	if (m["tables"].empty() && m["event"].empty()) {
		cerr << "[StartRoundModule] failed to start round no tables specified" << endl;
		m.add("status","1");
		return c->send(m);
	}

	if ( m["event"] != c->room()->name) {
		Room* rm = Server::room(m["event"]);
		if (rm == NULL) {
			cerr << "[StartRoundModule] no such room" << endl;
			m.add("status","1");
			return c->send(m);
		} else {
			m.add("status","0");
			rm->send_bot(m);
		}
	} else {
		list<string> tabs = split('|',m["tables"]);
		list<string>::iterator i;
		for (i = tabs.begin(); i != tabs.end(); ++i) {
			Room* rm = Server::room(*i);
			if (rm == NULL) {
				cerr << "[StartRoundModule] no such room " << *i << endl; 
			} else {
				rm->send_bot(m);
				rm->broadcast(m); // SEND TO ALL THE CLIENTS AS WELL
			}
		}
	}
	return true;
}

MODULE_PROCESS(LastHandModule)
{
	LOGIN_CHECK 

	if (m["tables"].empty() && m["event"].empty()) {
		cerr << "[LastHandModule] failed to start round no tables specified" << endl;
		m.add("status","1");
		return c->send(m);
	}

	if ( m["event"] != c->room()->name) {
		MSG_CHECK("table");
		Room* rm = Server::room(m["event"]);
		if (rm == NULL) {
			cerr << "[LastHandModule] no such room" << endl;
			m.add("status","1");
			return c->send(m);
		} else {
			m.add("status","0");
			rm->send_bot(m);
		}
	} else {
		list<string> tabs = split('|',m["tables"]);
		list<string>::iterator i;
		for (i = tabs.begin(); i != tabs.end(); ++i) {
			Room* rm = Server::room(*i);
			if (rm == NULL) {
				cerr << "[LastHandModule] no such room " << *i << endl; 
			} else {
				rm->send_bot(m);
				rm->broadcast(m); // SEND TO ALL THE CLIENTS AS WELL
			}
		}
	}
	return true;
}

MODULE_PROCESS(StartBreakModule)
{
	LOGIN_CHECK
	if (m["tables"].empty() && m["event"].empty()) {
		cerr << "[StartBreakModule] failed to start round no tables specified" << endl;
		m.add("status","1");
		return c->send(m);
	}

	if ( m["event"] != c->room()->name) {
		Room* rm = Server::room(m["event"]);
		if (rm == NULL) {
			cerr << "[StartBreakModule] no such room" << endl;
			m.add("status","1");
			return c->send(m);
		} else {
			m.add("status","0");
			rm->send_bot(m);
		}
	} else {
		list<string> tabs = split('|',m["tables"]);
		list<string>::iterator i;
		for (i = tabs.begin(); i != tabs.end(); ++i) {
			Room* rm = Server::room(*i);
			if (rm == NULL) {
				cerr << "[StartBreakModule] no such room " << *i << endl; 
			} else {
				rm->send_bot(m);
				rm->broadcast(m); // SEND TO ALL THE CLIENTS AS WELL
			}
		}
	}
	return true;
}

MODULE_PROCESS(TableSeatingModule)
{
	BOT_CHECK
	
	Table* t = c->room()->table();
	
	if (t == NULL || t->event == NULL || t->event->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	Tournament* tour  = t->event->tournament;

	list<tournament_seating>::iterator i;
	for (i = tour->seating.begin(); i != tour->seating.end(); ++i) {
		if (i->player == NULL) continue;
		Login* l = UserMap::getLogin(i->player->username + "@lobby");
		if (l != NULL) {
			Message msg;
			msg.add("msg","seat-player");
			msg.add("room", c->room()->name);
			msg.add("seat", string_of_int(i->seat));
			msg.add("purse", string_of_int(i->purse));
			l->sock->send(msg);
		}
		m.add(i->player->username + "@" + c->room()->name, 
			string_of_int(i->seat) + "!" 
			+ string_of_int(i->purse) + "!"
			+ string_of_int(i->position));
	}
	m.add("status","0");
	return c->send(m);	
}

MODULE_PROCESS(PadRoundsModule)
{
	BOT_CHECK
	MSG_CHECK("event")

	Event* e = Cache::find<Event>(m["event"]);
	if (e == NULL || e->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}	

	e->tournament->fabricate_rounds();

	Database* d = DBPool::grab();
	d->begin();
	if (! e->tournament->save(d) || ! d->commit()) {
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(TournamentTimerModule)
{
	BOT_CHECK

	Event* e = Cache::find<Event>(c->room()->name);
	if (e == NULL || e->tournament == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	m.add("current",string_of_int(time(NULL)));

	list<Table*>::iterator i;	
	for (i = e->tables.begin(); i != e->tables.end(); ++i) {
		if (*i == NULL) continue;
		Room* room = Server::room(string_of_Uint64((*i)->id));
		room->broadcast(m);
	}
	m.add("status","0");
	return c->send(m);
}

