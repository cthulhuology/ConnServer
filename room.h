// room.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// This is the chat room support for the server
// A room object tracks all of the sockets currently
// connected to a chat room.  Each room will handle
// broadcasting to its members.
//


#ifndef __HAVE_ROOM__
#define __HAVE_ROOM__

#include <iostream>
#include <list>

#include "esocket.h"
#include "message.h"
#include "data.h"

using namespace std;

class Room {
	public:
		Room(const string& s);
		~Room();
		void add(Socket* c);
		void remove(Socket* c);
		void add_bot(Socket* c);
		void remove_bot(Socket* c);
		bool announce(const string& o, const string& u, const string& a, const string& s);
		bool broadcast(Message& m);
		bool send_bot(Message& m);
		bool isBot(Socket* c);
		bool isMember(Socket* c);
		bool isEmpty();
		void disconnect_members();
		void members(Message& m);
		void bot_members(Message& m);
		void set_table(Table* t);
		void addAvatars(Message& m);
		Table* table();
		string name;
	private:
		Table* tab;
		Socket* bot;
		list<Socket*> mem;
};

#endif
