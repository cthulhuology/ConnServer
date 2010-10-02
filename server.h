// server.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// header for the connection server

#ifndef __HAVE_CONNSERVER__
#define __HAVE_CONNSERVER__

#include <map>
#include <list>
#include <iostream>
#include <signal.h>

#include "socket.h"
#include "message.h"
#include "room.h"
#include "object.h"
#include "event.h"
#include "cs.h"

using namespace std;

class Server {
	public:
	static bool init(int argc, char** argv);	
	static void finalize();
	static void serve();
	static bool broadcast(Message& m);
	static bool imessage(Message& m);
	static Room* addRoom(const string& s);
	static void removeRoom(const string& s);
	static Room* room(const string& s);
	static string room_list();
	static string name();
	static bool process(Socket* s);
	static CS* cs();
	static void addEvents(list<Event*>& el);
	static void removeEvent(Event* e);
	static bool saveEvents();
	static bool loadEvents();
	static void startEvents();
	static bool pendingEvents(Message& m);
	static Socket* get_watcher();
	static Socket* set_watcher(Socket* c);
	static bool send_watcher(Message& m);
	Room* lobby;

	protected:
	Server();
	Server(const Server&);
	Server& operator=(const Server&);
	~Server();
	
	private:
	static const int MAX_CLIENTS = 1000;
	static const int DB_CONNS = 20;
	static const int PONG_TIMEOUT = 10;
	static const int HEAD_START = 180; // 600;	// 10 minutes before official start time
	static Server* instance;
	CS* csptr;
	Socket* watcher;
	Socket* sock;
	map<string,Room*> rooms;
	list<Event*> events;
	bool pong_flag;
	bool reload_flag;
	static void quithandler(int s);
	static void reloadhandler(int s);
	static void alarmhandler(int s);
	void spawn(Socket* s);
	void awake(Socket* s);
};


#endif


