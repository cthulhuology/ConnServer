// league_modules.cc
//
//	Copyright (C) 2005 David J. Goehrig 
//
// modules for leagues and all that jazz

#include "league_modules.h"
#include "data.h"
#include "util.h"
#include "user_map.h"
#include "database.h"

BEGIN_REGISTRY
REGISTER(new-league, NewLeagueModule)
REGISTER(get-leagues, GetLeaguesModule)
REGISTER(join-league, JoinLeagueModule)
REGISTER(remove-member, RemoveMemberModule)
REGISTER(league-invite-email, LeagueInviteEmail)
REGISTER(league-invite-player, LeagueInvitePlayer)
REGISTER(league-roster, LeagueRosterModule)
REGISTER(commish-transfer, CommishTransferModule)
END_REGISTRY

MODULE_PROCESS(NewLeagueModule)
{
	LOGIN_CHECK
	MSG_CHECK("name")

	Login* login = c->login();

	string find_league = "SELECT id FROM leagues WHERE name = '" + m["name"] + "'";

	Database* d = DBPool::grab();
	d->begin();
	Result* res = d->query(find_league);
	if (res == NULL || ! res->success()) {
		cerr << "[NewLeagueModule] League already exists" << endl;
		m.add("status","1");
		d->rollback();
		delete res;
		DBPool::release(d);
		return c->send(m);
	} else if (res->rows > 0) {
		cerr << "[NewLeagueModule] League already exists" << endl;
		m.add("status","2");
		d->rollback();
		delete res;
		DBPool::release(d);
		return c->send(m);
	}
	delete res;
	
	League* league = League::alloc(d);
	league->name = m["name"];
	league->commish = login->player;
	league->num_members = 1;
	Member* member = Member::alloc(d);
	member->player = login->player;
	member->access = "";
	login->player->leagues.push_back(league);
	login->player->league_member_map[league->id] = member;
	league->members.push_back(member);
	
	if (! league->save(d)  || ! member->save(d) || ! login->player->save(d) ) {
		cerr << "[NewLeagueModule] failed to save league" << endl;
		login->player->leagues.pop_back();
		login->player->league_member_map.erase(league->id);
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);	
	}

	string league_insert = "INSERT INTO leagues (id,name) VALUES (" + string_of_Uint64(league->id) + ",'" + league->name + "')";

	res = d->query(league_insert);
	if (res == NULL || ! res->success() || res->rows > 0) {
		cerr << "[NewLeagueModule] League already exists" << endl;
		m.add("status","1");
		login->player->leagues.pop_back();
		d->rollback();
		d->rollback();
		delete res;
		DBPool::release(d);
		return c->send(m);
	}
	if (d->commit())  {
		m.add("status","0");
	} else {
		cerr << "[NewLeagueModule] Commit failed" << endl;
		login->player->leagues.pop_back();
		d->rollback();
		m.add("status","1");
	}
	delete res;
	DBPool::release(d);	
	return c->send(m);
}

MODULE_PROCESS(GetLeaguesModule)
{
	LOGIN_CHECK
	Login* login = c->login();
	list<League*>::iterator i;
	for (i = login->player->leagues.begin(); i != login->player->leagues.end(); ++i) {
		if ((*i)->commish == login->player) {
			m.add(string_of_Uint64((*i)->id),"c|" + (*i)->name);
		} else {
			m.add(string_of_Uint64((*i)->id),"m|" + (*i)->name);
		}
	}
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(JoinLeagueModule)
{
	LOGIN_CHECK
	if ( m["league"].empty() && m["invite"].empty()) {
		m.add("status","1");
		return c->send(m);
	}
	Login* login = c->login();
	if (! m["invite"].empty()) {
		Invite* invite = Cache::find<Invite>(m["invite"]);
		if (invite == NULL || invite->kind != Invite::LEAGUE) {
			cerr << "[JoinLeagueModule] invalid invite " << m["invite"] << endl;
			m.add("status","1");
			return c->send(m);
		}
		if (invite->recipient.player != login->player && login->player->details->email != invite->recipient.email.address) {
			cerr << "[JoinLeagueModule] invalid recipient " << m["invite"] << endl;
			m.add("status","2");
			return c->send(m);

		}
		Database* d = DBPool::grab();
		d->begin();
		League* league = invite->target.league;
		league->invites.remove(invite);
		Member* member = Member::alloc(d);
		member->player = login->player;
		member->access = "";
		login->player->leagues.push_back(league);
		login->player->league_member_map[league->id] = member;
		league->members.push_back(member);
		league->num_members += 1;
		invite->kind = Invite::ACCEPTED;
		if (! league->save(d) || ! member->save(d) || ! invite->save(d) || ! login->player->save(d)) {
			cerr << "[JoinLeagueModule] save failed" << endl;
			league->members.pop_back();
			league->num_members -= 1;
			login->player->league_member_map.erase(league->id);
			login->player->leagues.pop_back();
			m.add("status","1");
			d->rollback();
			DBPool::release(d);
			return c->send(m);
		}
		if (d->commit()) {
			m.add("status","0");
		} else {
			cerr << "[JoinLeagueModule] Commit failed" << endl;
			league->members.pop_back();
			league->num_members -= 1;
			login->player->leagues.pop_back();
			d->rollback();
			m.add("status","1");
		}
		DBPool::release(d);
	} else {
		League* league = Cache::find<League>(m["league"]);
		if (league == NULL) {
			cerr << "[JoinLeagueModule] invalid league" << m["league"] << endl;
			m.add("status","1");
			return c->send(m);
		}
		Database* d = DBPool::grab();
		d->begin();
		Member* member = Member::alloc(d);
		member->player = login->player;
		league->members.push_back(member);
		league->num_members += 1;
		member->player->leagues.push_back(league);
		if (! member->save(d) || ! league->save(d) || ! login->player->save(d)) {
			cerr << "[JoinLeagueModule] save failed" << endl;
			login->player->leagues.pop_back();
			league->members.pop_back();
			league->num_members -= 1;
			m.add("status","1");
			d->rollback();
			DBPool::release(d);
			return c->send(m);
		}
		if (d->commit()) {
			m.add("status","0");
		} else {
			cerr << "[JoinLeagueModule] commit failed" << endl;
			login->player->leagues.pop_back();
			league->members.pop_back();
			league->num_members -= 1;
			m.add("status","1");
			d->rollback();
		}
		DBPool::release(d);
	}
	return c->send(m);
}

MODULE_PROCESS(RemoveMemberModule)
{
	LOGIN_CHECK
	MSG_CHECK("league,un")
	
	string status = "1";
	Login* actor = c->login();
	League* league = Cache::find<League>(m["league"]);
	list<Member*>::iterator i;
	for (i = league->members.begin(); i != league->members.end(); ++i) {
		if (*i == NULL) continue;
		if ((*i)->player == NULL) continue;
		if ((*i)->player->username == m["un"]) {
			if ((*i)->player == actor->player || 
				actor->player == league->commish) {
				Member* mem = *i;
				league->members.remove(mem);
				Message msg;
				msg.add("msg","league-remove-notify");
				msg.add("league",league->name);
				Login* ml = UserMap::getLogin(m["un"] + "@lobby");
				if (ml != NULL) {
					ml->sock->send(msg);
				}
				Player* p = mem->player;
				cerr << "Removing league " << league->id << endl;
				p->league_member_map.erase(league->id);
				p->leagues.remove(league);
				map<UInt64,Member*>::iterator mi;
				for (mi = p->league_member_map.begin(); mi != p->league_member_map.end(); ++mi) {
					cerr << "Found league " << (*mi).first << endl;
				}
				Database* d = DBPool::grab();
				d->begin();
				if (! p->save(d) || ! league->save(d) || ! d->commit()) {
					d->rollback();
				} else {
					status = "0";
				}
				DBPool::release(d);
				break;
			}
		}
	}
	m.add("status",status);
	return c->send(m);	
}

MODULE_PROCESS(LeagueInviteEmail)
{
	LOGIN_CHECK
	MSG_CHECK("league,email")
	Login* login = c->login();
	League* league = Cache::find<League>(m["league"]);	
	if (league == NULL) {
		cerr << "[LeagueInviteEmail] invalid league " << m["league"] << endl;
		m.add("status","1");
		return c->send(m);
	}
	list<Member*>::iterator i;
	Invite* invite = NULL;
	Database* d = DBPool::grab();
	d->begin();
	for (i = league->members.begin(); i != league->members.end(); ++i) {
		if ((*i)->player != login->player) continue;
		invite = Invite::alloc(d);
		invite->kind = Invite::LEAGUE;
		invite->target.league = league;
		invite->dest = Invite::EMAIL;
		invite->recipient.email.address = m["email"];	
		invite->recipient.email.name = m["name"];
		invite->message = m["message"];
		invite->sender = login->player;
		league->invites.push_back(invite);
		login->player->invites.push_back(invite);
	}
	if (invite == NULL) {
		login->player->invites.pop_back();
		league->invites.pop_back();
		cerr << "[LeagueInviteEmail] sender not a member" << endl;
		m.add("status","1");
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	}

	Message msg("msg=send-invite&invite=" + string_of_Uint64(invite->id) + "&template=league_invite.txt");
	if (! Module::call("send-invite",c,msg)) {
		login->player->invites.pop_back();
		league->invites.pop_back();
		cerr << "[LeagueInviteEmail] failed to send email message" << endl;
		m.add("status","1");
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	}

	if (! invite->save(d) || ! league->save(d) || ! login->player->save(d)) {
		league->invites.pop_back();
		login->player->invites.pop_back();
		cerr << "[LeagueInviteEmail] failed to save" << endl;
		m.add("status","1");
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	}

	if (d->commit()) {
		m.add("status","0");
	} else {
		league->invites.pop_back();
		login->player->invites.pop_back();
		m.add("status","1");
		d->rollback();
	}	
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(LeagueInvitePlayer)
{
	LOGIN_CHECK
	MSG_CHECK("uid,league");
	Login* login = c->login();

	Login* recp = UserMap::getLogin(m["uid"]);
	if (recp == NULL || recp->player == NULL) {
		m.add("status","2");
		return c->send(m);
	}

	Player* player = recp->player;
	League* league = Cache::find<League>(m["league"]);
	if (league == NULL) {
		cerr << "[LeagueInvitePlayer] invalid league " << m["league"] << endl;
		m.add("status","1");
		return c->send(m);
	}

	list<Member*>::iterator i;
	Invite* invite = NULL;
	Database* d = DBPool::grab();
	d->begin();
	for (i = league->members.begin(); i != league->members.end(); ++i) {
		if ((*i)->player != login->player) continue;
		invite = Invite::alloc(d);
		invite->kind = Invite::LEAGUE;
		invite->dest = Invite::PLAYER;
		invite->target.league = league;
		invite->recipient.player = player;
		invite->message = m["message"];
		invite->recipient.player->invites.push_back(invite);
		invite->sender = login->player;
		league->invites.push_back(invite);
		login->player->invites.push_back(invite);
		break;	
	}
	if (invite == NULL) {
		cerr << "[LeagueInvitePlayer] sender not a member" << endl;
		m.add("status","1");
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	}
	if (! invite->save(d) || ! league->save(d) || ! invite->recipient.player->save(d) || ! login->player->save(d) || ! d->commit()) {
		cerr << "[LeagueInvitePlayer] failed to save" << endl;
		league->invites.pop_back();
		invite->recipient.player->invites.pop_back();
		login->player->invites.pop_back();	
		m.add("status","1");
		return c->send(m);
	}
	Message msg;
	msg.add("msg","leagueinvite");
	msg.add("league",string_of_Uint64(league->id));
	msg.add("name",league->name);
	msg.add("invite",string_of_Uint64(invite->id));
	msg.add("message",m["message"]);
	msg.add("un",login->player->username);
	recp->sock->send(msg);
	m.add("status","0");
	DBPool::release(d);
	return c->send(m);
}	


MODULE_PROCESS(LeagueRosterModule)
{
	MSG_CHECK("league")
	LOGIN_CHECK

	League* league = Cache::find<League>(m["league"]);
	if ( league == NULL || league->commish == NULL) {
		cerr << "[LeagueRosterModule] invalid league " << endl;
		m.add("status","1");
		return c->send(m);
	}
	list<Member*>::iterator i;
	for (i = league->members.begin(); i != league->members.end(); ++i ) {
		m.add((*i)->player->username, string("0!") + ((*i)->player == league->commish ? "c" : "m"));
	}
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(CommishTransferModule)
{
	MSG_CHECK("league,un");
	LOGIN_CHECK

	League* league = Cache::find<League>(m["league"]);
	if (league == NULL) {
		cerr << "[CommishTransferModule] failed to find league " << m["league"] << endl;
		m.add("status","1");
		return c->send(m);
	}
	Player* commish = league->commish;
	Player* new_commish = NULL;
	if (commish == NULL) {
		new_commish = c->login()->player;
	} else if (commish == c->login()->player) {
		list<Member*>::iterator i;
		for (i = league->members.begin(); i != league->members.end(); ++i) {
			if ((*i) == NULL || (*i)->player == NULL) continue;
			if ((*i)->player->username == m["un"]) {
				new_commish =  (*i)->player;
			}
		}
	}
	if (new_commish == NULL) {
		cerr << "[CommishTransferModule] Invalid commishioner transfer for league " << m["league"] << " to " << m["un"] << endl;
		m.add("status","2");
		return c->send(m);
	}
	league->commish = new_commish;	
	Database* d = DBPool::grab();
	d->begin();
	if (! league->save(d) || ! d->commit()) {
		d->rollback();
		league->commish = commish;
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	DBPool::release(d);	
	return c->send(m);
}


