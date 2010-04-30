// stats_modules.cc
//
//	Copyright (C) 2006  David J. Goehrig
//	All Rights Reserved
//


#include "stats_modules.h"
#include "data.h"
#include "database.h"
#include "user_map.h"
#include "room.h"

BEGIN_REGISTRY
REGISTER(populate-stats,PopulateStatsModule)
REGISTER(league-stats,LeagueStatsModule)
REGISTER(season-stats,SeasonStatsModule)
REGISTER(game-stats,GameStatsModule)
REGISTER(player-stats,PlayerStatsModule)
REGISTER(opponent-stats,OpponentStatsModule)
REGISTER(holdem-stats,HoldemStatsModule)
REGISTER(create-stats,CreateGameStats)
REGISTER(create-opp-stats,CreateOppStats) 
REGISTER(create-holdem-stats,CreateHoldemStats) 
REGISTER(season-ranks,SeasonRanksModule)
REGISTER(rank-players,RankPlayersModule)
END_REGISTRY

struct player_stats {
	int hands_seen;
	int hands_played;
	int events_played;
	int hands_won;
	int hands_lost;
	int chips_won;
	int chips_bought;
	int chips_banked;
	int chip_total;
	int points;
};

MODULE_PROCESS(PopulateStatsModule)
{
	BOT_CHECK
	MSG_CHECK("game")
	
	Game* g = Cache::find<Game>(m["game"]);
	Event* e = g->event;
	Season* s = e->season;
	if (s == NULL || s->league == NULL) {
		m.add("status","2");
		return c->send(m);
	}
	League* l = s->league;

	Database* d = DBPool::grab();
	d->begin();

	int i;
	for (i = 0; i < Game::MAX_SEATS; ++i) {
		Player* p = g->seats[i].player;
		if (p == NULL || s->hasStats(p)) continue;

		string psi = "INSERT INTO player_stats (league, season, player) VALUES (" + 
			string_of_Uint64(l->id) + ", " + 
			string_of_Uint64(s->id) + ", " + 
			string_of_Uint64(p->id) + ")";
		
		cerr << "[PopulateStateModule] " << psi << endl;
	
		string pri = "INSERT INTO player_ranking ( league, season, player) VALUES (" + 		
			string_of_Uint64(l->id) + ", " + 
			string_of_Uint64(s->id) + ", " + 
			string_of_Uint64(p->id) + ")";
		
		cerr << "[PopulateStateModule] " << pri << endl;

		Result* resa = d->query(psi);
		Result* resb = d->query(pri);
	
		if (resa == NULL || ! resa->success() || resb == NULL || ! resb->success()) {
			delete resa;
			delete resb;
			d->rollback();
			DBPool::release(d);
			m.add("status","1");
			return c->send(m);
		}
		s->stats.push_back(p);
		if (! s->save(d)) {
			d->rollback();
			DBPool::release(d);
			m.add("status","1");
			return c->send(m);
		}
		delete resa;
		delete resb;	
	}
	if (! d->commit()) {
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(LeagueStatsModule)
{
	LOGIN_CHECK
	MSG_CHECK("league")	
	
	League* l = Cache::find<League>(m["league"]);
	if (l == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	map<Player*,struct player_stats*> stat_map;

	Database* d = DBPool::grab();
	d->begin();
	list<Season*>::iterator j;
	for (j = l->seasons.begin(); j != l->seasons.end(); ++j) {
		if (*j == NULL) continue;
		string query = "SELECT player, hands_seen, hands_played, events_played, hands_won, hands_lost, chips_won, chips_bought, chips_banked, chip_total, points FROM player_stats WHERE season = " + string_of_Uint64((*j)->id);
		cerr << "[LeagueStatsModule] " << query << endl;
		Result* res = d->query(query);
		if (res == NULL || ! res->success()) {
			d->rollback();
			delete res;
			DBPool::release(d);
			m.add("status","1");
			return c->send(m);
		}
		for ( int i = 0; i < res->rows; ++i) {
			Player* p = Cache::find<Player>((*res)["player"][i]);
			cerr << "[LeagueStatsModule] player id " << (*res)["player"][i] << endl;
			if (p == NULL) continue;
			cerr << "[LeagueStatsModule]  player stats for " << p->username <<  " is " << stat_map[p] << endl;
			if (stat_map[p] == NULL)  {
				struct player_stats* st = new struct player_stats;
				st->hands_seen = int_of_string((*res)["hands_seen"][i]);
				st->hands_played = int_of_string((*res)["hands_played"][i]);
				st->events_played = int_of_string((*res)["events_played"][i]);
				st->hands_won = int_of_string((*res)["hands_won"][i]);
				st->hands_lost = int_of_string((*res)["hands_lost"][i]);
				st->chips_won = int_of_string((*res)["chips_won"][i]);
				st->chips_bought = int_of_string((*res)["chips_bought"][i]);
				st->chips_banked = int_of_string((*res)["chips_banked"][i]);
				st->chip_total = int_of_string((*res)["chip_total"][i]);
				st->points = int_of_string((*res)["points"][i]);
				stat_map[p] = st;
			} else {
				struct player_stats* st = stat_map[p];
				st->hands_seen += int_of_string((*res)["hands_seen"][i]);
				st->hands_played += int_of_string((*res)["hands_played"][i]);
				st->events_played += int_of_string((*res)["events_played"][i]);
				st->hands_won += int_of_string((*res)["hands_won"][i]);
				st->hands_lost += int_of_string((*res)["hands_lost"][i]);
				st->chips_won += int_of_string((*res)["chips_won"][i]);
				st->chips_banked += int_of_string((*res)["chips_banked"][i]);
				st->chip_total += int_of_string((*res)["chip_total"][i]);
				st->points += int_of_string((*res)["points"][i]);
			}
		}
		delete res;
	}
	map<Player*,player_stats*>::iterator k;
	for (k = stat_map.begin(); k != stat_map.end(); ++k) {
		if ( k->first == NULL || k->second == NULL) {
			cerr << "[LeagueStatsModule] null entry" << endl;
			continue;
		}
		cerr << "[LeagueStatsModule] player " << k->first->username << endl;
		m.add(k->first->username, 
			string_of_int(k->second->hands_seen) + "!" + 
			string_of_int(k->second->hands_played) + "!" +
			string_of_int(k->second->events_played) + "!" +
			string_of_int(k->second->hands_won) + "!" +
			string_of_int(k->second->hands_lost) + "!" +
			string_of_int(k->second->chips_won) + "!" +
			string_of_int(k->second->chips_bought) + "!" +
			string_of_int(k->second->chips_banked) + "!" +
			string_of_int(k->second->chip_total) + "!" +
			string_of_int(k->second->points));
		delete k->second;
	}
	d->commit();
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);

}

MODULE_PROCESS(SeasonStatsModule)
{
	LOGIN_CHECK
	MSG_CHECK("season")	

	Season* s = Cache::find<Season>(m["season"]);
	if (s == NULL || s->league == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	string query = "SELECT player, hands_seen, hands_played, events_played, hands_won, hands_lost, chips_won, chips_bought, chips_banked, chip_total, points FROM player_stats WHERE season = " + string_of_Uint64(s->id);

	Database* d = DBPool::grab();
	d->begin();
	Result* res = d->query(query);
	if (res == NULL || ! res->success()) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	for ( int i = 0; i < res->rows; ++i) {
		Player* p = Cache::find<Player>((*res)["player"][i]);
		if (p == NULL) continue;
		m.add(p->username,
			(*res)["hands_seen"][i] + "!" +
			(*res)["hands_played"][i] + "!" +
			(*res)["events_played"][i] + "!" +
			(*res)["hands_won"][i] + "!" +
			(*res)["hands_lost"][i] + "!" +
			(*res)["chips_won"][i] + "!" +
			(*res)["chips_bought"][i] + "!" +
			(*res)["chips_banked"][i] + "!" +
			(*res)["chip_total"][i] + "!" +
			(*res)["points"][i]);
	}
	delete res;
	d->commit();
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(PlayerStatsModule)
{
	LOGIN_CHECK
	MSG_CHECK("player")

	Database* d = DBPool::grab();
	d->begin();

	string query = "SELECT player_id FROM login WHERE username = '" + m["player"] + "'";
	
	cerr << "[PlayerStatsModule] " << query << endl;

	Result* res = d->query(query);
	if (res == NULL || ! res->success() || res->rows == 0) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	Player* p = Cache::find<Player>((*res)["player_id"][0]);
	if (p == NULL) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	string sel_query = "SELECT season, hands_seen, hands_played, events_played, hands_won, hands_lost, chips_won, chips_bought, chips_banked, chip_total, points FROM player_stats WHERE player = " + (*res)["player_id"][0];

	delete res;

	cerr << "[PlayerStatsModule] " << sel_query << endl;

	res = d->query(sel_query);
	if (res == NULL || ! res->success()) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	for (int i = 0; i < res->rows; ++i) {
		Season* s = Cache::find<Season>((*res)["season"][i]);
		if (s == NULL || s->league == NULL) continue;
		m.add(string_of_Uint64(s->id),
			s->name + "!" +
			s->league->name + "!" +
			(*res)["hands_seen"][i] + "!" + 
			(*res)["hands_played"][i] + "!" + 
			(*res)["events_played"][i] + "!" + 
			(*res)["hands_won"][i] + "!" + 
			(*res)["hands_lost"][i] + "!" + 
			(*res)["chips_won"][i] + "!" + 
			(*res)["chips_bought"][i] + "!" + 
			(*res)["chips_banked"][i] + "!" + 
			(*res)["chip_total"][i] + "!" + 
			(*res)["points"][i]);
	}

	d->commit();
	delete res;
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(GameStatsModule)
{
	LOGIN_CHECK
	MSG_CHECK("season,player,hands")
	
	Database* d = DBPool::grab();
	d->begin();

	string query = "SELECT player_id FROM login WHERE username = '" + m["player"] + "'";
	
	cerr << "[PlayerStatsModule] " << query << endl;

	Result* res = d->query(query);
	if (res == NULL || ! res->success() || res->rows == 0) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	Season* s = Cache::find<Season>(m["season"]);

	Player* p = Cache::find<Player>((*res)["player_id"][0]);
	if (p == NULL || s == NULL ) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}
	delete res;

	query = "SELECT game, time, starting_hand, final_hand, stage, showdown, pot, pot_type, all_in, head_up, position, initial_action, initial_bet FROM game_stats WHERE season = " + string_of_Uint64(s->id) + " AND player = " + string_of_Uint64(p->id) + " AND starting_hand_code = " + m["hands"] ;

	cerr << "[GameStatsModule] " << query << endl;

	res = d->query(query);
	if ( res == NULL || ! res->success()) {
		delete res;
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	for (int i = 0; i < res->rows; ++i ) {
		m.add((*res)["time"][i], 
			(*res)["game"][i] + "!" +
			(*res)["starting_hand"][i] + "!" +
			(*res)["final_hand"][i] + "!" +
			(*res)["stage"][i] + "!" +
			(*res)["showdown"][i] + "!" +
			(*res)["pot"][i] + "!" +
			(*res)["pot_type"][i] + "!" +
			(*res)["all_in"][i] + "!" +
			(*res)["head_up"][i] + "!" +
			(*res)["position"][i] + "!" +
			(*res)["initial_action"][i] + "!" +
			(*res)["initial_bet"][i]);
	}

	d->commit();
	delete res;
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(OpponentStatsModule)
{
	LOGIN_CHECK
	MSG_CHECK("season")

	Season* s = Cache::find<Season>(m["season"]);
	Player* p = c->login()->player;

	if (s == NULL || p == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	Database* d = DBPool::grab();
	d->begin();

	string query = "SELECT opp, hands_seen, hands_played, hands_won, hands_lost, showdown, folds_to_my_raise, my_folds_to_raise FROM opp_stats WHERE player = " + string_of_Uint64(p->id) + " AND season = " + string_of_Uint64(s->id);

	cerr << "[OpponentStatsModule] " << query << endl;

	Result* res = d->query(query);
	
	if ( res == NULL || ! res->success()) {
		delete res;
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	for (int i = 0; i < res->rows; ++i ) {
		Player* p = Cache::find<Player>((*res)["opp"][i]);
		if (p == NULL) continue;
		m.add(p->username, 
			(*res)["hands_seen"][i] + "!" + 
			(*res)["hands_played"][i] + "!" + 
			(*res)["hands_won"][i] + "!" + 
			(*res)["hands_lost"][i] + "!" + 
			(*res)["showdown"][i] + "!" + 
			(*res)["folds_to_my_raise"][i] + "!" + 
			(*res)["my_folds_to_raise"][i]);
	}
	d->commit();
	delete res;
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(HoldemStatsModule)
{
	LOGIN_CHECK
	MSG_CHECK("season")

	Season* s = Cache::find<Season>(m["season"]);
	Player* p = c->login()->player;
	if ( s == NULL || p == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	string query = "SELECT starting_hand, suited, off_suit, saw_flop, won, lost, flop_top_pair, flop_set, flop_two_pair, flop_straight, flop_flush, flop_boat, flop_foak, flop_straight_flush, preflop_raise, preflop_raise_amt FROM holdem_stats WHERE season = " + string_of_Uint64(s->id) + " AND player = " + string_of_Uint64(p->id);


	cerr << "[HoldemStatsModule] " << query << endl;

	Database* d = DBPool::grab();
	d->begin();

	Result* res = d->query(query);
	if ( res == NULL || ! res->success()) {
		delete res;
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	for (int i = 0; i < res->rows; ++i) {
		cerr << "[HoldemStatsModule]" << endl;
		cerr << (*res)["starting_hand"][i]  << endl;
		cerr << (*res)["suited"][i] << endl;
		cerr << (*res)["off_suit"][i] << endl;
		cerr << (*res)["saw_flop"][i] << endl;
		cerr << (*res)["won"][i] << endl;
		cerr << (*res)["lost"][i] << endl;
		cerr << (*res)["flop_top_pair"][i] << endl;
		cerr << (*res)["flop_set"][i] << endl;
		cerr << (*res)["flop_two_pair"][i] << endl;
		cerr << (*res)["flop_straight"][i] << endl;
		cerr << (*res)["flop_flush"][i] << endl;
		cerr << (*res)["flop_boat"][i] << endl;
		cerr << (*res)["flop_foak"][i] << endl;
		cerr << (*res)["flop_straight_flush"][i] << endl;
		cerr << (*res)["preflop_raise"][i] << endl;
		cerr << (*res)["preflop_raise_amt"][i] << endl;
		m.add((*res)["starting_hand"][i],
			(*res)["suited"][i] + "!" + 
			(*res)["off_suit"][i] + "!" + 
			(*res)["saw_flop"][i] + "!" + 
			(*res)["won"][i] + "!" + 
			(*res)["lost"][i] + "!" + 
			(*res)["flop_top_pair"][i] + "!" + 
			(*res)["flop_set"][i] + "!" + 
			(*res)["flop_two_pair"][i] + "!" + 
			(*res)["flop_straight"][i] + "!" + 
			(*res)["flop_flush"][i] + "!" + 
			(*res)["flop_boat"][i] + "!" + 
			(*res)["flop_foak"][i] + "!" + 
			(*res)["flop_straight_flush"][i] + "!" + 
			(*res)["preflop_raise"][i] + "!" + 
			(*res)["preflop_raise_amt"][i] );
	}
	d->commit();
	delete res;
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(CreateGameStats)
{
	BOT_CHECK

	Table* t = c->room()->table();
	if (t == NULL || t->current == NULL || t->current->event == NULL || t->current->event->season == NULL || t->current->event->season->league == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	Game* g = t->current;
	Event* e = g->event;
	Season* s = e->season;
	League* l = s->league;

	Database* d = DBPool::grab();
	d->begin();

	for (int i = 0; i < Table::MAX_SEATS; ++i) {
		Player* p = g->seats[i].player;
		if (p == NULL) continue;
		string str = m[string_of_Uint64(p->id)];
		cerr << "[CreateGameStats] data: " << str << endl;
		list<string> v = split('!',str);
		if (v.size() != 20) {
			cerr << "[CreateGameStats] invalid data: " << str << endl;
			continue;
		}
		list<string>::iterator j = v.begin();
		string hands_seen = *j; 	// 0
		string hands_played = *(++j);	// 1
		string hands_won = *(++j);	// 2
		string hands_lost = *(++j);	// 3
		string chips_won = *(++j);	// 4
		string chips_bought = *(++j);	// 5
		string chips_banked = *(++j);	// 6
		string chips_total = *(++j);	// 7
		string points = *(++j);		// 8
		string starting_hand = *(++j);	// 9
		string final_hand = *(++j);	//10
		string stage = *(++j);		//11
		string showdown = *(++j);	//12
		string pot = *(++j);		//13
		string pot_type = *(++j);	//14
		string all_in = *(++j);		//15
		string head_up = *(++j);	//16
		string position = *(++j);	//17
		string initial_action = *(++j);	//18
		string initial_bet = *(++j);	//19
		//	       2  3  4  5  6   7   8    9  10  J   Q   K   A
		int primes[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41 }; 
		UInt64 starting_hand_code = 1;
		int i;
		for (i = 0; i < starting_hand.size(); ++i) {
			if ( starting_hand[i] >= 'a' && starting_hand[i] < 'n' ) starting_hand_code *= primes[starting_hand[i] - 'a'];
			if ( starting_hand[i] >= 'n' && starting_hand[i] <= 'z' ) starting_hand_code *= primes[starting_hand[i] - 'n'];
			if ( starting_hand[i] >= 'A' && starting_hand[i] < 'N' ) starting_hand_code *= primes[starting_hand[i] - 'A'];
			if ( starting_hand[i] >= 'N' && starting_hand[i] <= 'Z' ) starting_hand_code *= primes[starting_hand[i] - 'N'];
		}
		cerr << "Starting hand Code: " << starting_hand_code << " starting hand: " << starting_hand << endl;

		int t = time(NULL);
		
		string in_query = "INSERT INTO game_stats (season,event,game,player,game_type, time, starting_hand,starting_hand_code,final_hand,stage,showdown,pot,pot_type,all_in,head_up,position,initial_action,initial_bet) values (" 
			+ string_of_Uint64(s->id) + ","  // season
			+ string_of_Uint64(e->id) + ","	// event
			+ string_of_Uint64(g->id) + ","  // game
			+ string_of_Uint64(p->id) + "," // player
			+ g->game + "," // game type
			+ string_of_int(t) + ",'" // timke
			+ starting_hand + "',"  // starting_hand
			+ string_of_int(starting_hand_code) + ",'"  // starting_hand_code
			+ final_hand + "','"  // final_hand
			+ stage + "'," // stage
			+ showdown + ","  // showdown
			+ pot + ",'" // pot
			+ pot_type + "'," // pot type
			+ all_in + ",'" // all_in
			+ head_up + "'," // head_up
			+ position + ",'" // position
			+ initial_action + "'," // initial_action
			+ initial_bet + ")"; // initial_bet
		
		cerr << "[CreateGameStats] " << in_query << endl;
		
		string up_query = "UPDATE player_stats SET hands_seen = hands_seen + " + hands_seen
			+ ", hands_played = hands_played + " + hands_played + 
			+ ", events_played = (select count(distinct event) from game_stats where player = " + string_of_Uint64(p->id) + 
			+ "), hands_won = hands_won + " + hands_won + 
			+ ", hands_lost = hands_lost + " + hands_lost + 
			+ ", chips_won = chips_won + " + chips_won + 
			+ ", chips_bought  = chips_bought  + " + chips_bought + 
			+ ", chips_banked = chips_banked + " + chips_banked + 
			+ ", chip_total = chips_banked - chips_bought + " + chips_total + 
			+ ", points = points + " + points +
			" WHERE player = " + string_of_Uint64(p->id) +
			" AND season = " + string_of_Uint64(s->id);

		cerr << "[CreateGameStats] " << up_query << endl;

		Result* resa = d->query(in_query);
		Result* resb = d->query(up_query);
		if (resa == NULL || ! resa->success() || resb == NULL || ! resb->success()) {
			delete resa;
			delete resb;
			d->rollback();
			DBPool::release(d);
			m.add("status","1");
			return c->send(m);
		}
	}
	d->commit();
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(SeasonRanksModule)
{
	LOGIN_CHECK
	MSG_CHECK("season")
	
	Season* s = Cache::find<Season>(m["season"]);
	if (s == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	string query = "SELECT player, rank, best, worst FROM player_ranking WHERE season = " + string_of_Uint64(s->id) + " ORDER BY rank";

	cerr << "[SeasonRanksModule] " << query << endl;

	Database* d = DBPool::grab();
	d->begin();

	Result* res = d->query(query);
	if (res == NULL || ! res->success()) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	for (int i = 0; i < res->rows; ++i) {
		Player* p = Cache::find<Player>((*res)["player"][i]);
		if (p == NULL) continue;
		m.add(p->username, (*res)["rank"][i] + "!" + (*res)["best"][i]
			+ "!" + (*res)["worst"][i]);
	}

	d->commit();
	delete res;
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(RankPlayersModule)
{
	BOT_CHECK
	MSG_CHECK("season")

	Season* s = Cache::find<Season>(m["season"]);
	if (s == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	Database* d = DBPool::grab();
	d->begin();

	cerr << "[RankPlayersModule] players ranked by " << s->ranking << endl;

	string query = "SELECT ps.player, ps.chip_total, ps.points, pr.rank, pr.best, pr.worst FROM player_stats ps, player_ranking pr WHERE ps.player = pr.player AND ps.season = " + string_of_Uint64(s->id) + ( s->ranking == "0" ? " ORDER BY ps.chip_total" : " ORDER BY ps.points");

	cerr << "[RankPlayersModule] " << query << endl;

	Result* res = d->query(query);
	if (res == NULL || ! res->success()) {
		d->rollback();
		delete res;
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}

	for ( int i = 0; i < res->rows; ++i) {
		string q = "UPDATE player_ranking SET rank = " 
			+ string_of_int(i+1)
			+ ( int_of_string((*res)["best"][i]) > (i+1) ? ", best = " + string_of_int(i+1) : "")
			+ ( int_of_string((*res)["worst"][i]) < (i+1) ? ", worst = " + string_of_int(i+1) : "")
			+ " WHERE season = " + string_of_Uint64(s->id)
			+ " AND player = " + (*res)["player"][i];

		cerr << "[RankPlayersModule] " << q << endl;
		Result* r = d->query(q);
		if (r == NULL || ! r->success()) {
			d->rollback();
			delete res;
			delete r;
			DBPool::release(d);
			m.add("status","1");
			return c->send(m);
		}
		delete r;	
	}	
	d->commit();
	delete res;
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);	
}

MODULE_PROCESS(CreateOppStats)
{
	BOT_CHECK

	Table* t = c->room()->table();
	if (t == NULL || t->current == NULL || t->current->event == NULL ||
		t->current->event->season == NULL || 
		t->current->event->season->league == NULL) {
		m.add("status","1");
		return c->send(m);
	}	

	Game* g = t->current;
	Event* e = t->current->event;
	Season* s = t->current->event->season;
	Database*  d = DBPool::grab();
	d->begin();

	for (int i = 0; i <  Table::MAX_SEATS; ++i) {
		cerr << "[CreateOppStats] seat" << i << endl;
		if (g->seats[i].player == NULL) continue;
		Player* p = g->seats[i].player;
		cerr << "[CreateOppStats] player " << p->username << endl;	
		string str = m[string_of_Uint64(p->id)];
		cerr << "[CreateOppStats] data: " << str << " for id " << string_of_Uint64(p->id) << endl;
		list<string> ov = split('!',str);
		for (list<string>::iterator ovi = ov.begin();ovi != ov.end(); ++ovi) {
			list<string> v = split(',',*ovi);
			if (v.size() != 8) {
				cerr << "[CreateOppStats] invalid data: " << *ovi << endl;
				continue;
			}
			list<string>::iterator vi = v.begin();
			Player* op = Cache::find<Player>(*vi);
			if (op == NULL) {
				cerr << "[CreateOppStats] could not fild player " << *vi << endl;
				continue;
			}

			string hands_seen = *(++vi);
			string hands_played = *(++vi);
			string hands_won = *(++vi);
			string hands_lost = *(++vi);
			string showdown = *(++vi);
			string folds_to_my_raise = *(++vi);
			string my_folds_to_raise = *(++vi);

			string query;

			if (s->hasOppStats(p,op)) {
				query = "UPDATE opp_stats SET hands_seen = hands_seen + " + hands_seen + 
					", hands_played = hands_played + " + hands_played + 
					", hands_won = hands_won + " + hands_won + 
					", hands_lost = hands_lost + " + hands_lost + 
					", showdown = showdown + " + showdown + 
					", folds_to_my_raise = folds_to_my_raise + " + folds_to_my_raise + 
					", my_folds_to_raise = my_folds_to_raise + " + my_folds_to_raise + 
					" WHERE season = " + string_of_Uint64(s->id) + 
					" AND player = " + string_of_Uint64(p->id) + 
					" AND opp = " + string_of_Uint64(op->id);
				cerr << "[CreateOppStats] " << query << endl;
			} else {
				query =  "INSERT INTO opp_stats ( season, player, opp, hands_seen, hands_played, hands_won, hands_lost, showdown, folds_to_my_raise, my_folds_to_raise ) VALUES (" + 
					string_of_Uint64(s->id) + "," + 
					string_of_Uint64(p->id) + "," + 
					string_of_Uint64(op->id) + "," + 
					hands_seen + "," +
					hands_played + "," +
					hands_won + "," +
					hands_lost + "," +
					showdown + "," +
					folds_to_my_raise + "," +
					my_folds_to_raise+ ")";
				cerr << "[CreateOppStats] " << query << endl;
				s->opp_stats.push_back(PVP(p,op)); // add the PVP entry
			}
			Result* res = d->query(query);
			if (res == NULL || ! res->success()) {
				cerr << "[CreateOppStats] Failed " << query << " error: " << d->error() << endl;	
				d->rollback();
				delete res;
				DBPool::release(d);
				m.add("status","1");
				return c->send(m);
			} else {
				cerr << "[CreateOppStats] did " << query << endl;
			}
			delete res;
		}
	}
	if (! s->save(d) || ! d->commit()) {
		cerr << "[CreateOppStats] Failed" << endl;
		m.add("status","1");
	} else {
		cerr << "[CreateOppStats] Success" << endl;
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(CreateHoldemStats)
{
	BOT_CHECK

	Table* t = c->room()->table();
	if (t == NULL || t->current == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	Game* g = t->current;
	if (g == NULL || g->event== NULL || g->event->season == NULL || g->event->season->league == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	Event* e = g->event;
	Season* s = e->season;

	Database* d = DBPool::grab();
	d->begin();

	for (int i = 0; i != Table::MAX_SEATS; ++i) {
		cerr << "[CreateHoldemStats] seat " << i << endl;
		Player* p = g->seats[i].player;
		if (p == NULL) continue;
		cerr << "[CreateHoldemStats] player " << p->username << endl;
		string str = m[string_of_Uint64(p->id)];
		cerr << "[CreateHoldemStats] data: " << str << " for id " << string_of_Uint64(p->id) << endl;
		list<string> v = split('!',str);
		if (v.size() != 16) {
			cerr << "[CreateHoldemStats] invalid data: " << str << endl;
			continue;
		}
		list<string>::iterator vi = v.begin();

		string starting_hand = *vi;		//0
		string suited = *(++vi);		//1
		string off_suit = *(++vi);		//2
		string saw_flop = *(++vi);		//3
		string won = *(++vi);			//4
		string lost = *(++vi);			//5
		string flop_top_pair = *(++vi);		//6
		string flop_set = *(++vi);		//7
		string flop_two_pair = *(++vi);		//8
		string flop_straight = *(++vi);		//9
		string flop_flush = *(++vi);		//10
		string flop_boat = *(++vi);		//11
		string flop_foak = *(++vi);		//12
		string flop_straight_flush = *(++vi);	//13
		string preflop_raise = *(++vi);		//14
		string preflop_raise_amt = *(++vi);	//15

		string query;

		if (s->hasHoldemStats(p,starting_hand)) {
		
			query = "UPDATE holdem_stats SET suited = suited + " + suited +
				", off_suit = off_suit + " + off_suit +
				", saw_flop = saw_flop + " + saw_flop + 
				", won = won + " + won +
				", lost = lost + " + lost +
				", flop_top_pair = flop_top_pair + " + flop_top_pair +
				", flop_set = flop_set + " + flop_set +
				", flop_two_pair = flop_two_pair + " + flop_two_pair +
				", flop_straight = flop_straight + " + flop_straight +
				", flop_flush = flop_flush + " + flop_flush +
				", flop_boat = flop_boat + " + flop_boat +
				", flop_foak = flop_foak + " + flop_foak +
				", flop_straight_flush = flop_straight_flush + " + flop_straight_flush +
				", preflop_raise = preflop_raise + " + preflop_raise +
				", preflop_raise_amt = preflop_raise_amt + " + preflop_raise_amt +
				" WHERE season = " + string_of_Uint64(s->id) +
				" AND player = " + string_of_Uint64(p->id) +
				" AND starting_hand = '" + starting_hand + "'";

			cerr << "[CreateHoldemStats] " << query << endl;

		} else {
			
			query = "INSERT INTO holdem_stats (season, player, starting_hand, suited, off_suit, saw_flop, won, lost, flop_top_pair, flop_set, flop_two_pair, flop_straight, flop_flush, flop_boat, flop_foak, flop_straight_flush, preflop_raise, preflop_raise_amt ) VALUES (" + 
				string_of_Uint64(s->id) + "," +
				string_of_Uint64(p->id) + ",'" +
				starting_hand + "'," +
				suited + "," +
				off_suit + "," +
				saw_flop + "," +
				won + "," +
				lost + "," +
				flop_top_pair + "," +
				flop_set + "," +
				flop_two_pair + "," +
				flop_straight + "," +
				flop_flush + "," +
				flop_boat + "," +
				flop_foak + "," +
				flop_straight_flush + "," +
				preflop_raise + "," +
				preflop_raise_amt + ")";

			cerr << "[CreateHoldemStats] " << query  << endl;
				s->holdem_stats.push_back(HSP(p,starting_hand));
		}

		Result* res = d->query(query);
		if (res == NULL || ! res->success()) {
			cerr << "[CreateHoldemStats] Failed " << query << " error: " << d->error() << endl;
			d->rollback();
			delete res;
			DBPool::release(d);
			m.add("status","1");
			return c->send(m);
 		} 
		delete res;
	}
	if (! s->save(d) || ! d->commit()) {
		cerr << "[CreateHoldemStats] Failed to save season / commit" << endl;
		m.add("status","1");
	} else {	
		cerr << "[CreateHoldemStats] Success" << endl;
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}


