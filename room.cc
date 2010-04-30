// room.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// This handles chat room support for the server
//

#include "room.h"
#include "user_map.h"
#include "server.h"
#include "util.h"

Room::Room(const string& s) : name(s)
{
	cerr << "Room::Room creating room " << name << endl;
	bot = NULL;
	tab = NULL;
}

Room::~Room()
{
	cerr << "Room::~Room Destroying room " << name << endl;
}

void Room::add(Socket* c)
{
	if (! isMember(c)) {
		mem.push_back(c);
		c->set_room(this);
	}
}

void Room::remove(Socket* c)
{
	if (isMember(c))  {
		mem.remove(c);
		c->set_room(NULL);
		if (isEmpty()) {
			Server::removeRoom(name);
		}
	}
}

void Room::add_bot(Socket* c)
{
	c->set_room(this);
	bot = c;
}

void Room::remove_bot(Socket* c)
{
	if (bot == c) {
		c->set_room(NULL);
		bot = NULL;
		Server::removeRoom(name);
	}
}

bool Room::send_bot(Message& m)
{
//	cerr << "Room::send_bot called bot " << bot << endl;
	if(bot != NULL && bot->connected()) {
//		cerr << "Room::send_bot sending " << m.dump() << endl;
		return bot->send(m);
	} 
	return false;
}

bool Room::announce(const string& o, const string& u, const string& a, const string& s)
{
	Message m;
	m.add("msg",o);
	m.add("status",s);
	m.add("avatar",a);
	m.add("uid",u);
	broadcast(m);
	send_bot(m);
	return true;
}

bool Room::broadcast(Message& m)
{
	bool retval = true;
//	cerr << "Room::broadcast has " << mem.size() << " members " << endl;
	list<Socket*>::iterator c;
	for (c = mem.begin(); c != mem.end(); ++c) {
//		cerr << "Room::broadcast = " << m.dump() << endl;
		if ((*c)->connected()) {
			if (! (*c)->send(m)) {
				Login* l = UserMap::getLogin(*c);
				if ( l != NULL) {
					cerr << "Room::broadcast failed for user " << l->uid << endl;
				}
				retval = false;
			}
		}
	}
	// ADDING BROADCAST MESSAGE TO THE REPLAY 
	if (tab != NULL && tab->current != NULL && ! tab->current->done && tab->current->replay != NULL) {
//		cerr << "[Room::braodcast] saving " << m.dump() << endl;
		if (m["msg"] == "chat") {
			m.remove("msg");
			m.add("msg","replay-chat");
		}
		if (m["msg"] == "state") {
			addAvatars(m);	
		}
		tab->current->replay->add(m);
	} else {
//		cerr << "Not saving replay" << endl;
//		cerr << "tab? " << (tab == NULL ? "NULL" : "yes") << endl;
//		cerr << "current? " << (tab == NULL ? "NULL" :( tab->current == NULL ? "NULL" : "yes")) << endl;
	}
	return retval;
}

bool Room::isBot(Socket* c)
{
	return bot == c;
}

bool Room::isMember(Socket* c)
{
	list<Socket*>::iterator r = find(mem.begin(),mem.end(),c);
	return *r == c;
}

bool Room::isEmpty()
{
	return mem.empty() && bot == NULL;
}

void Room::members(Message& m)
{
	list<Socket*>::iterator r;
	for (r = mem.begin();r != mem.end(); ++r) {
		Login* l = UserMap::getLogin(*r);
		if (l != NULL && l->player != NULL) {
			m.add(l->uid,l->player->avatar);	
		}
	}
}

void Room::bot_members(Message& m)
{
	list<Socket*>::iterator r;
	for (r = mem.begin();r != mem.end(); ++r) {
		Login* l = UserMap::getLogin(*r);
		if (l != NULL && l->player != NULL) {
			m.add(string_of_Uint64(l->player->id),l->uid);	
		}
	}
}

void Room::disconnect_members()
{
	list<Socket*>::iterator i;
	for (i = mem.begin(); i != mem.end(); ++i) {
		(*i)->set_room(NULL);
	}
	mem.clear();	
}

void Room::set_table(Table* t)
{
	tab = t;
}

Table* Room::table()
{
	return tab;
}

void Room::addAvatars(Message& m)
{
	list<Socket*>::iterator i;
	for(i = mem.begin(); i  != mem.end(); ++i ) {
		if (*i == NULL || (*i)->login() == NULL || (*i)->login()->player == NULL) continue;
		m.add((*i)->login()->uid,(*i)->login()->player->avatar);
	}
}
