// poker_modules.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// Modules for poker games
//

#include "poker_modules.h"
#include "user_map.h"
#include "server.h"

BEGIN_REGISTRY
REGISTER(newgame,NewGameModule)
REGISTER(end-game,EndGameModule)
REGISTER(end-event,EndEventModule)
REGISTER(get-game-state,GetGameStateModule)
REGISTER(set-game-state,SetGameStateModule)
REGISTER(seat,SeatModule)
REGISTER(start,StartModule)
REGISTER(dealer,DealerModule)
REGISTER(reveal,RevealModule)
REGISTER(turn,TurnModule)
REGISTER(draw,DrawModule)
REGISTER(cards,CardsModule)
REGISTER(pass,PassModule)
REGISTER(discard,DiscardModule)
REGISTER(order,OrderModule)
REGISTER(choose,ChooseModule)
REGISTER(show,ShowModule)
REGISTER(blinds,BlindsModule)
REGISTER(ante,AnteModule)
REGISTER(fold,FoldModule)
REGISTER(call,CallModule)
REGISTER(bet,BetModule)
REGISTER(raise,RaiseModule)
REGISTER(check,CheckModule)
REGISTER(auto-fold,AutoFoldModule)
REGISTER(leavetable,LeaveTableModule)
REGISTER(winner,WinnerModule)
REGISTER(state,StateModule)
REGISTER(replay-state,ReplayStateModule)
REGISTER(community,CommunityModule)
REGISTER(rabbit,RabbitModule)
REGISTER(done,DoneModule)
REGISTER(timer,TimerModule)
REGISTER(change-state,ChangeStateModule)
END_REGISTRY 

MODULE_PROCESS(NewGameModule)
{
	BOT_CHECK

	Database* d = DBPool::grab();
	d->begin();

	Game* game = Game::alloc(d);
	game->game = m["game"];

	game->table = c->room()->table();
	
	if (game->table == NULL) {
		cerr << "[NewGameModule] invalid table" << endl;
		m.add("status","1");
		return c->send(m);
	}

	game->event = game->table->event;
	game->table->games.push_back(game);
	game->table->current = game;
	game->done = false;
	
	game->replay = Replay::alloc(d);
		
	memcpy(game->seats,game->table->seats,sizeof(game->seats));

	if ( ! game->save(d) || ! game->table->save(d) || ! d->commit()) {
		cerr << "[NewGameModule] failed to save state" << endl;
		game->table->games.pop_back();
		d->rollback();
		m.add("status","1");
	} else {
		int i;
		for (i = 0; i < Game::MAX_SEATS; ++i) {
			if (game->seats[i].player == NULL) continue;
			cerr << "[NewGameModule] " << game->seats[i].player->username << "@" << c->room()->name << endl;
			m.add( game->seats[i].player->username + "@" + c->room()->name, string_of_Uint64(game->seats[i].player->id));	
		}
		m.add("status","0");
		m.add("gid",string_of_Uint64(game->id));
	}
	Message msg;
	msg.add("msg","populate-stats");
	msg.add("game",string_of_Uint64(game->id));
	Module::call("populate-stats",c,msg);
	DBPool::release(d);
	c->room()->broadcast(m); // added so gene knows when to start the game
	return c->send(m);
}

MODULE_PROCESS(GetGameStateModule)
{
	


}

MODULE_PROCESS(SetGameStateModule)
{
	if (! c->isBot()) return false;

	MSG_CHECK("gid,pots");

	Game* game = Cache::find<Game>(m["gid"]);

	game->pots = split('|',m["pots"]);
	
	int i;
	for (i = 0; i < Game::MAX_SEATS; ++i) {
		string sts = m["seat" + string_of_int(i)];
		string rts = m["result" + string_of_int(i)];
		if (! sts.empty()) {
			game->seats[i].decode(sts);	
		}
		if (! rts.empty()) {
			game->results[i].decode(rts);
		}
	}

	Database* d = DBPool::grab();
	d->begin();

	if (! game->save(d) || ! d->commit()) {
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
	}	
	DBPool::release(d);
	return c->send(m);
}


MODULE_PROCESS(SeatModule) {
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[SeatModule] no such user " << m["uid"] << endl;
		}
		if (m["status"] == "0") {
			Table* table = c->room()->table();
			list<string>::iterator i;
			list<Player*>::iterator p;
			int seat_no = int_of_string(m["seat"]) - 1;
			bool found = false;
			table->seats[seat_no].player = login->player;
			table->seats[seat_no].seat = seat_no;
			login->seat = seat_no;
			Database* d = DBPool::grab();
			d->begin();	
			if (! table->save(d) || ! d->commit()) {
				d->rollback();
			}
			DBPool::release(d);
		}
		login->sock->send(m);
	} else {
		if (c->room() == Server::room("lobby"))  {
			cerr << "[SeatModule] failed user " << c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	} 
	return true;
}

MODULE_PROCESS(StartModule) { 
	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[StartModule] failed user " << c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			 return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(DealerModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(RevealModule) {
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[RevealModule] no such user " << m["uid"] << endl;
			return true;
		}	
		login->sock->send(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[RevealModule] failed user " << c->login()->uid <<  "in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(TurnModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	}
	return false;
}


MODULE_PROCESS(DrawModule) 
{
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[DrawModule] no such user"  << m["uid"] << endl;
			return true;
		}
		login->sock->send(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(CardsModule) {
	if (! c->isBot()) {
		if(c->room() == Server::room("lobby"))  {
			cerr << "[CardsModule] user " << c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);	
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	} else {
		Login* login = UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[CardsModule] no such user "  << m["uid"] << endl;
			return true;
		}
		login->sock->send(m);
	}
	return true;
}

MODULE_PROCESS(PassModule) {
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[PassModule] no such user " << m["uid"] << endl;
			return true;
		}
		login->sock->send(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(DiscardModule) {
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[DiscardModule] no such user " << m["uid"] << endl;
			return true;
		}
		login->sock->send(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(OrderModule) {
	if (c->isBot()) {
		Login* login = UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[OrderModule] no such user " << m["uid"] << endl;
			return true;
		}
		login->sock->send(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(ChooseModule) {
	if (c->isBot()) {
		Login* login =  UserMap::getLogin(m["uid"]);
		if (login == NULL) {
			cerr << "[ChooseModule] no such user " << m["uid"] << endl;
			return true;
		}
		login->sock->send(m);
		return true;
	} else {
		Room* r = c->room();
		if (r == Server::room("lobby")) {
			cerr << "[ChooseModule] user " << c->login()->uid <<  " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		r->send_bot(m);
		return true;
	}
}


MODULE_PROCESS(ShowModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	} else {
		m.add("uid",c->login()->uid);
		if (c->room() == Server::room("lobby")) {
			cerr << "[ShowModule] user " << c->login()->uid <<  " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		if (m["to"] == "all") {
			c->room()->broadcast(m);
		} else {
			Login* to = UserMap::getLogin(m["to"]);
			if (to == NULL || ! to->sock->send(m)) {
				cerr << "[ShowModule] user " << m["to"] << " failed to recieve message" << endl;
				m.add("status","1");
			} else {
				m.add("status","0");	
			}
			return c->send(m);
		}
	}
	return false;
}

MODULE_PROCESS(BlindsModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(AnteModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(FoldModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[FoldModule] user " <<  c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(CallModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[CallModule] user " <<  c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(BetModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[BetModule] user " << c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(RaiseModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[RaiseModule] user " <<  c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}


MODULE_PROCESS(CheckModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[CheckModule] user " <<  c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}


MODULE_PROCESS(AutoFoldModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	}
	return false;
}


MODULE_PROCESS(LeaveTableModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if (c->room() == Server::room("lobby")) {
			cerr << "[LeaveTableModule] failed user " << c->login()->uid <<  " in lobby" << endl;
			m.add("status","1");
			return c->send(m);
		}
		Table* table = c->room()->table();
		if (table == NULL) {
			m.add("status","1");
			return c->send(m);
		}
		if (table->event->tournament == NULL) { // leaving table in tournaments does not remove you from the table
			int seat_no = c->login()->seat;
			int purse = table->seats[seat_no].purse;
			table->seats[seat_no].player = NULL;
			table->seats[seat_no].seat = -1;
			table->seats[seat_no].purse = 0;
			c->login()->seat = -1;
			Database* d = DBPool::grab();
			d->begin();
			if (! table->save(d) || ! d->commit()) {
				table->seats[seat_no].player = c->login()->player;
				table->seats[seat_no].seat = seat_no;
				table->seats[seat_no].purse = purse;
				d->rollback();
				m.add("status","1");
				DBPool::release(d);
				return c->send(m);
			}
			DBPool::release(d);
		}
		m.add("uid", c->login()->uid);
		m.add("player",string_of_Uint64(c->login()->player->id));
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(WinnerModule) {
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	} 
	return false;
}

MODULE_PROCESS(StateModule)
{
	if (c->isBot()) {
		cerr << "[StateModule] sending " << m.dump() << endl;
		c->room()->broadcast(m);	
		return true;
	}
	return false;
}

MODULE_PROCESS(ReplayStateModule)
{ if (c->isBot()) {
		Table* t = c->room()->table();
		if (t != NULL && t->current != NULL && ! t->current->done && t->current->replay != NULL) {
			t->current->replay->add(m);
		}
		return true;	
	}
	return false;
}

MODULE_PROCESS(CommunityModule)
{
	if (c->isBot()) {
		c->room()->broadcast(m);	
		return true;
	}
	return false;
}

MODULE_PROCESS(RabbitModule)
{
	if (c->isBot()) {
		c->room()->broadcast(m);
		return true;
	}
	return false;
}

MODULE_PROCESS(DoneModule)
{
	if (c->isBot()) {
		Table* t = c->room()->table();
		if ( t != NULL && t->current != NULL) {
			t->current->done = true;
			Database* d = DBPool::grab();
			d->begin();
			if (! t->current->save(d) || ! t->current->replay->save(d) || ! d->commit()) {
				d->rollback();
				m.add("status","1");
			}
			DBPool::release(d);
		}
		c->room()->broadcast(m);
	}
}

MODULE_PROCESS(TimerModule)
{
	BOT_CHECK
	
	m.add("current",string_of_int(time(NULL)));
	c->room()->broadcast(m);

	return true;	
}

MODULE_PROCESS(ChangeStateModule)
{
	LOGIN_CHECK
	MSG_CHECK("game")

	if (c->isBot()) {
		c->room()->broadcast(m);
	} else {
		if(c->room() == Server::room("lobby"))  {
			cerr << "[CardsModule] user " << c->login()->uid << " in lobby" << endl;
			m.add("status","1");
			return c->send(m);	
		}
		m.add("uid",c->login()->uid);
		c->room()->send_bot(m);
	}
	return true;
}

MODULE_PROCESS(EndGameModule)
{
	BOT_CHECK
	MSG_CHECK("game")

	Room* rm = Server::room(m["game"]);
	if (rm == NULL) return true;

	rm->send_bot(m);
	return true;	
}

MODULE_PROCESS(EndEventModule)
{
	LOGIN_CHECK

	if (! c->isBot()) {
		Table* t = c->room()->table();	
		Event* e = t->event;
		if (c->login()->player == e->owner) {
			list<Table*>::iterator i;
			for (i = e->tables.begin(); i != e->tables.end(); ++i) {
				Room* rm = Server::room(string_of_Uint64((*i)->id));
				if (rm == NULL) continue;
				rm->send_bot(m);
			}
			m.add("status","0");	
		} else {
			cerr << "[EndEventModule] failed " << c->login()->player->username << " invoked but event owned by " << e->owner->username << endl;
			m.add("status","1");
		}
		return c->send(m);
	} 
	return true;
}


