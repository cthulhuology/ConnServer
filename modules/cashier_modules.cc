// cashier_modules.cc
//
//	Copyright (C) 2005 David J. Goehrig
//
//

#include "cashier_modules.h"
#include "database.h"
#include "data.h"
#include "util.h"
#include "user_map.h"
#include "server.h"


BEGIN_REGISTRY
REGISTER(putontable,PutOnTableModule)
REGISTER(rebuy,RebuyModule)
REGISTER(addon,AddonModule)
REGISTER(cashier,CashierModule)
REGISTER(bank,BankModule)
REGISTER(tally-season,TallySeasonModule)
REGISTER(tally-event,TallyEventModule)
END_REGISTRY

MODULE_PROCESS(PutOnTableModule) {
	LOGIN_CHECK
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		Table* table = c->room()->table();
		if (login == NULL || table == NULL) {
			cerr << "[PutOnTableModule] no such user or table " << m["uid"] << " - " << c->room()->name << endl;
			return true;
		}
		if (login->seat < 0 
		|| login->seat >= Table::MAX_SEATS 
		|| table->seats[login->seat].player != login->player) {
			cerr << "[PutOnTableModule] invalid player" << endl;
			m.add("status","1");
		} else {
			table->seats[login->seat].purse += int_of_string(m["chips"]);
		}
		login->sock->send(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[PutOnTableModule] failed user " << c->login()->uid <<  " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(RebuyModule) {
	LOGIN_CHECK
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		Table* table = c->room()->table();
		if (login == NULL || table == NULL) {
			cerr << "[RebuyModule] no such user or table " << m["uid"] << " - " << c->room()->name << endl;
			return true;
		}
		if (login->seat < 0 
		|| login->seat >= Table::MAX_SEATS 
		|| table->seats[login->seat].player != login->player) {
			cerr << "[RebuyModule] invalid player" << endl;
			m.add("status","1");
		} else {
			table->seats[login->seat].purse += int_of_string(m["chips"]);
		}
		login->sock->send(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[RebuyModule] failed user " << c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(AddonModule) {
	LOGIN_CHECK
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		Table* table = c->room()->table();
		if (login == NULL || table == NULL) {
			cerr << "[AddonModule] no such user or table " << m["uid"] << " - " << c->room()->name << endl;
			return true;
		}
		if (login->seat < 0 
		|| login->seat >= Table::MAX_SEATS 
		|| table->seats[login->seat].player != login->player) {
			cerr << "[AddonModule] invalid player" << endl;
			m.add("status","1");
		} else {
			table->seats[login->seat].purse += int_of_string(m["chips"]);
		}
		login->sock->send(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[AddonModule] failed user " << c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(CashierModule)
{
	if (c->isBot()) {
		Player* player = NULL;
		if (! m["uid"].empty()) {
			Login* login = UserMap::getLogin(m["uid"]);
			player = login->player;
		} else if (! m["player"].empty()) {
			player = Cache::find<Player>(m["player"]);
		}	
		if (player == NULL) {
			cerr << "[CashierModule] failed to find player " << m["player"] << endl;
			m.add("status","1");
			return c->send(m);
		}
		Database* d = DBPool::grab();
		d->begin();
		Table* table = c->room()->table();
		Event* event = table->event;
		Season* s = event->season;
		Cashier* cashier = Cashier::alloc(d);
		cashier->player = player;
		cashier->event = event;
		cashier->pool = (event->options->value == 0 ? Cashier::FREEBIE : (event->season == NULL ? Cashier::LEAGUE : Cashier::SEASON ));
		cashier->kind = (event->tournament == NULL ? Cashier::CASH : Cashier::TOURNEY);
		cashier->cost = (cashier->kind == Cashier::TOURNEY ? event->tournament->cost : int_of_string(m["chips"]));
		cashier->chips = (cashier->kind == Cashier::TOURNEY ? event->tournament->chips: int_of_string(m["chips"]));
		player->cashiers.push_back(cashier);
		event->cashiers.push_back(cashier);

		if ( s != NULL && s->league != NULL) {
			string query = "UPDATE player_stats SET chips_bought = chips_bought + "  + m["chips"] + " WHERE player = " + string_of_Uint64(player->id)
				+ " AND season = " + string_of_Uint64(s->id);	
	
			cerr << "[CashierModule] " << query << endl;
			
			Result* res = d->query(query);
			if (res == NULL || ! res->success()) {
				cerr << "[CashierModule] Failed to cashier chips to stats " << d->error() << endl;
				delete res;
				d->rollback();
				DBPool::release(d);
				m.add("status","1");
				return c->send(m);
			}
			delete res;
		}

		if (! cashier->save(d) || ! player->save(d) || ! event->save(d) || ! d->commit() ) {
			player->cashiers.pop_back();
			event->cashiers.pop_back();
			d->rollback();
			DBPool::release(d);
			m.add("status","1");	
			return c->send(m);
		} 
		DBPool::release(d);
		m.add("status","0");
		return c->send(m);
	}
	return false;
}

MODULE_PROCESS(BankModule)
{
	if (c->isBot()) {
		Table* table = c->room()->table();
		Player* player = NULL;
		if (! m["player"].empty()) {
			player = Cache::find<Player>(m["player"]);
		} else if (! m["uid"].empty()) {
			Login* login = UserMap::getLogin(m["uid"]);
			if (login == NULL) {
				cerr << "[BankModule] player " << m["uid"] << " not logged in" << cerr;
				m.add("status","1");
			} else {
				player = login->player;
			}
		} 
		if ( player == NULL || table == NULL ) {
			cerr << "[BankModule] failed no such player or room " << m["player"] <<  ", " << c->room()->name << endl;
			m.add("status","1");
			return c->send(m);
		}
		Event* event = table->event;
		Season* s = event->season;
		Database* d = DBPool::grab();
		d->begin();
		Bank* bank = Bank::alloc(d);
		bank->player = player;
		bank->event = table->event;
		bank->chips = int_of_string(m["chips"]);
		bank->pool = (event->options->value == 0 ? Bank::FREEBIE : (event->season == NULL ? Bank::LEAGUE : Bank::SEASON ));
		player->banks.push_back(bank);
		event->banks.push_back(bank);
		
		if (s != NULL && s->league != NULL) {
			string query = "UPDATE player_stats SET chips_banked = chips_banked + "  + m["chips"] + " WHERE player = " + string_of_Uint64(player->id)
				+ " AND season = " + string_of_Uint64(s->id);	
	
			cerr << "[BankModule] " << query << endl;
			
			Result* res = d->query(query);
			if (res == NULL || ! res->success()) {
				cerr << "[BankModule] Failed to bank chips to stats " << d->error() << endl;
				delete res;
				d->rollback();
				DBPool::release(d);
				m.add("status","1");
				return c->send(m);
			}
			delete res;
		}

		if (! bank->save(d) || ! player->save(d) || ! event->save(d) || ! d->commit()) {
			player->banks.pop_back();
			event->banks.pop_back();
			d->rollback();
			DBPool::release(d);
			m.add("status","1");
			return c->send(m);
		}
		DBPool::release(d);
		m.add("status","0");
		return c->send(m);
	}
	return false;
}

MODULE_PROCESS(TallySeasonModule)
{
	LOGIN_CHECK
	if (m["season"].empty()) {
		cerr << "[TallySeasonModule] failed no season specified" << endl;
		m.add("status","1");
		return c->send(m);
	}
	Season* season = Cache::find<Season>(m["season"]);
	if (season == NULL ) {
		cerr << "[TallySeasonModule] no such season " << m["season"]  << endl;
		m.add("status","1");
		return c->send(m);
	}
	map<Player*,int> purchases;
	map<Player*,int> winnings;
	list<Event*>::iterator i;
	for (i = season->events.begin(); i != season->events.end(); ++i) {
		list<Cashier*>::iterator ci;
		list<Bank*>::iterator b;
		if (*i != NULL) {
			for (ci = (*i)->cashiers.begin(); ci != (*i)->cashiers.end(); ++ci) {
				purchases[(*ci)->player] += (*ci)->cost;
			}
			for (b = (*i)->banks.begin(); b != (*i)->banks.end(); ++b) {
				winnings[(*b)->player] += (*b)->chips;
			}
		}	
	}
	map<Player*,int>::iterator j;
	for (j = purchases.begin(); j != purchases.end(); ++j) {
		Player* p = j->first;
		if (p != NULL) {
			m.add(p->username, string_of_int(j->second) + "!" + string_of_int( winnings[p] ));
		}
	}
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(TallyEventModule)
{
	MSG_CHECK("event")
	LOGIN_CHECK

	Event* event = Cache::find<Event>(m["event"]);
	if (event == NULL ) {
		cerr << "[TallyEventModule] no such event " << m["event"]  << endl;
		m.add("status","1");
		return c->send(m);
	}
	map<Player*,int> purchases;
	map<Player*,int> winnings;
	list<Cashier*>::iterator ci;
	list<Bank*>::iterator b;
	for (ci = event->cashiers.begin(); ci != event->cashiers.end(); ++ci) {
		purchases[(*ci)->player] += (*ci)->cost;
	}
	for (b = event->banks.begin(); b != event->banks.end(); ++b) {
		winnings[(*b)->player] += (*b)->chips;
	}
	map<Player*,int>::iterator j;
	for (j = purchases.begin(); j != purchases.end(); ++j) {
		Player* p = j->first;
		if (p != NULL) {
			m.add(p->username, string_of_int(j->second) + "!" + string_of_int( winnings[p] ));
		}
	}
	m.add("status","0");
	return c->send(m);
}
