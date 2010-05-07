// friends_modules.cc
//
//	Copyright (C) 2005 David J. Goehrig
//	All Rights Reserved
//

#include "friends_modules.h"
#include "database.h"
#include "object.h"
#include "util.h"
#include "user_map.h"

using namespace std;

BEGIN_REGISTRY
REGISTER(add-friend,AddFriendModule)
REGISTER(accept-friend,AcceptFriendModule)
REGISTER(remove-friend,RemoveFriendModule)
REGISTER(update-friend,UpdateFriendModule)
REGISTER(friends,FriendsModule)
END_REGISTRY

MODULE_PROCESS(AddFriendModule)
{
	MSG_CHECK( "uid,category,rating,personality" )
	LOGIN_CHECK

	if (c->isBot()) { return true; }
	Player* p = c->login()->player;
	Login* l = UserMap::getLogin(m["player"]);

	if (l == NULL || p == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	Database* d = DBPool::grab();
	d->begin();
	
	Friend* f = Friend::alloc(d);
	f->player = l->player;
	f->category = m["category"];
	f->rating = m["rating"];
	f->personality = m["personality"];
	f->notes = m["notes"];	

	if (! f->save(d) || ! d->commit()) {
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}
	DBPool::release(d);

	Message msg;
	msg.add("msg","request-friend");
	msg.add("text",m["text"]);
	msg.add("avatar",c->login()->player->avatar);
	msg.add("player",string_of_Uint64(c->login()->player->id));
	msg.add("friend",string_of_Uint64(f->id));
	l->sock->send(msg);
	m.add("status","3");
	return c->send(m);
}

MODULE_PROCESS(AcceptFriendModule)
{
	MSG_CHECK( "uid,accept,friend,category,rating,personality" )
	LOGIN_CHECK

	if (c->isBot()) { return true; }
	Player* p = c->login()->player;
	Login* l = UserMap::getLogin(m["player"]);
	Friend* of = Cache::find<Friend>(m["friend"]);

	if (l == NULL || p == NULL || of == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	
	Message msg;
	msg.add("msg","add-friend");
	msg.add("player",string_of_Uint64(c->login()->player->id));
	if ( m["accept"] != "0" ) {
		msg.add("status","2");		
		l->sock->send(msg);
		m.add("status","0");
		return c->send(m);
	}
	
	Database* d = DBPool::grab();
	d->begin();

	Friend* f = Friend::alloc(d);
	f->player = l->player;
	f->category = m["category"];
	f->rating = m["rating"];
	f->personality = m["personality"];
	f->notes = m["notes"];	

	p->friends.push_back(f);
	l->player->friends.push_back(of);

	if (! f->save(d) || ! p->save(d) || ! l->player->save(d) || ! d->commit()) {
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		msg.add("status","1");
		l->sock->send(msg);
		return c->send(m);
	}
	DBPool::release(d);
	msg.add("status","0");
	m.add("status","0");
	l->sock->send(msg);
	return c->send(m);
}

MODULE_PROCESS(RemoveFriendModule)
{
	LOGIN_CHECK
	if (c->isBot()) { return true; }
	Player* p = c->login()->player;

	list<Friend*>::iterator i;	
	Friend* f = NULL;
	for ( i = p->friends.begin(); i != p->friends.end(); ++i) {
		if (*i == NULL) continue;
		if ((*i)->player == NULL) continue;
		if (m["un"] == (*i)->player->username || m["un"] == ((*i)->player->username + "@lobby")) {
			f = (*i);
			break;
		}
	}
	if ( f == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	Player* op = f->player;
	Friend* of = NULL;		
	for ( i = op->friends.begin(); i != op->friends.end(); ++i) {
		if (*i == NULL) continue;
		if ((*i)->player == NULL) continue;
		if (p == (*i)->player) {
			of = (*i);
			break;
		}
	}
	if (of == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	op->friends.remove(of);
	p->friends.remove(f);

	Database* d = DBPool::grab();
	d->begin();
	if (! p->save(d) || ! op->save(d) || ! d->commit()) {
		d->rollback();
		DBPool::release(d);
		op->friends.push_back(of);
		p->friends.push_back(f);
		m.add("status","1");
		return c->send(m);	
	}
	DBPool::release(d);
	
	m.add("status","0");
	m.add("remover",p->username);
	Login* l = UserMap::getLogin(op->username + "@lobby");
	if (l != NULL) {
		l->sock->send(m);
	}
	return c->send(m);
}

MODULE_PROCESS(UpdateFriendModule)
{
	MSG_CHECK ( "un,category,rating,personality" )
	LOGIN_CHECK

	Player* p = c->login()->player;
	Friend* f = NULL;

	list<Friend*>::iterator i;
	for (i = p->friends.begin(); i != p->friends.end(); ++i) {
		if (*i == NULL) continue;
		if ((*i)->player != NULL && (*i)->player->username == m["un"]) {
			f = (*i);
		}
	}
	if (f == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	Database* d = DBPool::grab();
	d->begin();
	f->category = m["category"];
	f->rating = m["rating"];
	f->personality = m["personality"];
	f->notes = m["notes"];
	if (! f->save(d) || ! d->commit()) {
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(FriendsModule)
{
	LOGIN_CHECK
	Player* p = c->login()->player;
	if (p == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	
	list<Friend*>::iterator i;
	for (i = p->friends.begin();  i != p->friends.end(); ++i ) {
		if (*i == NULL) continue;
		if ((*i)->player == NULL) continue;
		Login* log = UserMap::getLogin((*i)->player->username + "@lobby");	
		string online = "1";
		if (log == NULL) {
			online = "0";
		} 
		m.add((*i)->player->username, (*i)->player->avatar + "!"+ (*i)->category + "!" + (*i)->rating + "!" + (*i)->personality + "!" + online + "!" + (*i)->notes);
	}
	m.add("status","0");
	return c->send(m);
}
