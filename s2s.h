// s2s.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// Server to Server Connections
// This file contains the basic mechanism for s2s messaging and is intended
// only as a low volume discovery tool.  It is assumed that each server is
// able to access the database.  s2s messaging is for trackign client 
// connections and aiding in load-balancing, not directing traffic between
// servers.  As the client is supposed to be minimally intelligent, it is
// easier to tell a client "bob is at 742 evergreen terrace" and have the
// client go there, than to relay every damn message the client wants to send
// to bob.

#include <string>
#include <map>
#include <iostream>
using namespace std;

#include "message.h"
#include "socket.h"

class ServerMap {
	public:
		bool init(const string& s);
		bool finalize(ServerMap* sm);
		bool add(const string& h, const string& p);
		bool remove(const string& s);
		bool send(const string& s, Message& m);
		Socket* find(const string& s);
		bool discover(vector<string>& v);
	protected:
		ServerMap();
		ServerMap(const string& s);
		ServerMap(const ServerMap&);
		~ServerMap();
		ServerMap& operator=(const ServerMap&);
	private:
		static bool initialized;
		static ServerMap* servermap;
		map<string,Socket*> svrs;
		const string& name;
};
