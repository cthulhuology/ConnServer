// server.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
#include "server.h"
#include "database.h"
#include "user_map.h"
#include "module.h"
#include "interp.h"
#include "env.h"
#include "pool.h"

Server* Server::instance = NULL;

bool Server::init(int argc, char** argv)
{
	if (Env::env["CSDBCONN"].empty()) {
		cerr << "Server::init failed no CSDBCONN environment variable" << endl;
		return false;
	}
	if (Env::env["CSID"].empty()) {
		cerr << "Server::init failed no CSID environment variable" << endl;
		return false;

	}
	if (instance == NULL) {
		Interpreter::init(argc,argv);
		instance = new Server();
		// Load the pending events from the server.
		if (! loadEvents()) {
			cerr << "Server::init failed to load events" << endl;
			return false;
		}
	};
	return true;
}
void Server::finalize()
{
	delete instance;
	Server::instance = NULL;
}

void Server::serve()
{
	int i,ready;
	Message m;

	m.add("msg","pong");
	m.add("status","0");

	alarm(PONG_TIMEOUT);
	for (;;) {
		// Time based functions
		if (instance->pong_flag) {
			instance->pong_flag = false;
			cerr << "Broadcasting pong" << endl;
			alarm(0);
			int flushed  = Pool::flush();
			cerr << "[Server::serve] flushed " << flushed << " connections" << endl;
			Server::broadcast(m);
			Server::startEvents();
			alarm(PONG_TIMEOUT);
		}
		if (instance->reload_flag) {
			instance->reload_flag = false;
			 Registry::load();
		}
		time_t t = time(NULL);
		cerr << "[Server::server] before wait time " <<  ctime(&t);
		ready = Pool::wait();
		t = time(NULL);
		cerr << "[Server::server] after wait time " <<  ctime(&t);
		if (ready > 0) {
			cerr << "Ready sockets " << ready << endl;
		}
		for (i = 0; i < ready; i++) {
			Socket* s = Pool::find(i);
			if (s == instance->sock) { // Server socket accept
				Socket* client = instance->sock->open();
			} else {	
				if (! Server::process(s)) {
					s->disconnect();
				}
			}
		}
	}
}

bool Server::process(Socket* s) 
{
	Message m;
	string buf;	

	while (s->read_msg(m,buf)) {
		cerr << "[Server::process message [" << m.dump() << "] string [" << buf << "]" << endl;
		if (m["msg"].empty()) continue;
		if (! Module::call(m["msg"],s,m)) {
			cerr << "Server::process failed module process " << m.dump() << endl;
			return false;
		}
	}
	return s->connected();
}

bool Server::broadcast(Message& m)
{
	bool retval = true;
	string s = m.build();
	list<Socket*> socks = UserMap::sockets();
	list<Socket*>::iterator i;
	for (i = socks.begin();i != socks.end(); ++i) {
		if ( ! (*i)->connected() || (s.size() > (*i)->out(s))) {
			cerr << "Server::broadcast failed socket " << s << endl;
			retval = false;
		}
	}
	return retval;
}

bool Server::imessage(Message& m)
{
	string uid = m["to"];
	if (uid.empty()) return false;
	Login* l = UserMap::getLogin(uid);
	if (l == NULL || l->sock == NULL) return false;
	return l->sock->send(m);
}

Room* Server::addRoom(const string& s) 
{
	if (instance == NULL) {
		cerr << "Server::addRoom server not initialized" << endl;
		return NULL;
	}
	if (NULL == instance->rooms[s]) {
		return instance->rooms[s] = new Room(s);
	}
	return instance->rooms[s];
}

void Server::removeRoom(const string& s)
{
	if (instance == NULL) {
		cerr << "Server::addRoom server not initialized" << endl;
		return;
	}
	Room* r = instance->rooms[s];
	instance->rooms.erase(s);
	if (r) {
		cerr << "Deleting room" << r << endl;
		delete r;
	}
}

Room* Server::room(const string& s)
{
	if (instance == NULL) {
		cerr << "Server::addRoom server not initialized" << endl;
		return NULL;
	}
	if (s == "lobby") return instance->lobby;
	return instance->rooms[s];
}

string Server::room_list()
{
	if (instance == NULL) {
		cerr << "Server::room_list server not initialized" << endl;
		return "";
	}
	string retval = "";
	map<string,Room*>::iterator i;	
	for (i = instance->rooms.begin(); i != instance->rooms.end(); ++i) {
		if (i->second != NULL) {
			if (retval == "") {
				retval = (i->second)->name;
			} else {
				retval += "!" + (i->second)->name;
			}
		}
	}
	return retval;
}

string Server::name()
{
	return instance->csptr->name;
}

Server::Server ()
{
	pong_flag = false;
	watcher = NULL;

	signal(SIGHUP,reloadhandler);
	signal(SIGQUIT,quithandler);
	signal(SIGINT,quithandler);
	signal(SIGTERM,quithandler);
	signal(SIGSEGV,quithandler);
	signal(SIGALRM,alarmhandler);

	cerr << "Server::Server connecting to database" << endl;
	if (! DBPool::init(Env::env["CSDBCONN"],DB_CONNS)) {
		cerr << "Server::Server failed to initialize DBPool" << endl;
		exit(1);
	}

	cerr << "Server::Server creating cache" << endl;
	if (! Cache::create(Env::env["CSCACHEID"])) {
		cerr << "Server::Server failed to initialize Cache" << endl;
		exit(1);
	}

	cerr << "Sever::Server loading CS settings: " << Env::env["CSID"] << endl;
	csptr = Cache::find<CS>(Env::env["CSID"]);
	if ( csptr == NULL) {
		cerr << "Server::Server no such CSID " << Env::env["CSID"] << endl;
		exit(1);
	}

	cerr << "Server::Server initializing socket pool" << endl;
	if (! Pool::init(MAX_CLIENTS)) {
		cerr << "Server::Server failed to initialize Pool" << endl;
		exit(1);
	}

	cerr << "Sever::Server initializing user mappings" << endl;
	if (! UserMap::init()) {
		cerr << "Server::Server failed to initiaize UserMap" << endl;
		exit(1);
	}

	cerr << "Server::Server creating socket" << endl;
	sock = Socket::create();
	if (! sock->monitor(csptr->port)) {
		cerr << "Server::Server Failed to monitor" << endl;	
		exit(1);
	}

	cerr << "Server::Server initializing registry" << endl;
	if (! Registry::init()) {
		cerr << "Server::Server Failed to initialize registry" << endl;	
		exit(1);
	}

	cerr << "Server::Server creating lobby" << endl;
	lobby = new Room("lobby");
	rooms["lobby"] = lobby;
}

Server::~Server () 
{
	// Stop accepting connections
	Socket::finalize(sock);

	// Remove all clients
	cerr << "Server::~Server killing all clients" << endl;
	list<Socket*> v = UserMap::sockets();
	list<Socket*>::iterator vi;
	for (vi = v.begin(); vi != v.end(); ++vi) {
		cerr << "Server::~Server killing client " << *vi << endl;
		(*vi)->disconnect();
	}

	// Save Events
	saveEvents();

	// Remove the UserMap
	cerr << "Server::~Server finalizing the UserMap" << endl;
	UserMap::finalize();

	// Free up the pool
	cerr << "Server::~Server finalizing the socket pool" << endl;
	Pool::finalize();

	// Remove chatrooms
	cerr << "Server::~Server removing chat rooms" << endl;
	if (rooms.size()) {
		map<string,Room*>::iterator i;
		for (i = rooms.begin();i != rooms.end(); ++i) {
			Room* r = (*i).second;
			delete r;
		}
	}

	cerr << "Server::~Server clearing object cache" << endl;
	if (!Cache::fini()) {
		cerr << "Server::~Server faild to finalize cache" << endl;
	}
	
	// Disconnect the DB
	cerr << "Server::~Server shutting down db connections" << endl;	
	if (!DBPool::finalize()) {
		cerr << "Server::~Server failed to finalize databases" << endl;
	}

	// Clear out the registry
	cerr << "Server::~Server unregistering modules" << endl;
	if (!Registry::finalize()) {
		cerr << "Server::~Server failed to finalize registry" << endl;
	}

}

void Server::reloadhandler(int s)
{
	cerr << "Got signal " << s << " reloading registry" << endl;
	instance->reload_flag = true;
}

void Server::quithandler(int s)
{
	cerr << "Got signal " << s << " finalizing server" << endl;
	finalize();
	cerr << "Done" << endl;
	exit(0);
}

void Server::startEvents()
{
	static Module* mod = Registry::find("start-event");
	time_t t = time(NULL);
	if (mod == NULL) {
		cerr << "[Server::startEvents] no such module start-event" << endl;
		return;
	}
go_again:
	if (instance->events.size() <= 0 || instance->watcher == NULL) return;
	EventTimer et = instance->events.front();
	cerr << "[Server::startEvents] time " <<  ctime(&t);
	cerr << "[Server::startEvents] next event " << ctime(&et.time) << endl;
	if (et.time - HEAD_START  <= t) {
		if ( et.event == NULL || t - et.time > 3600) {
			cerr << "[Server::startEvents] Stale event found in queue" << endl;
			instance->events.pop_front();
			goto go_again;
		}
		Message m;
		m.add("msg","start-event");
		m.add("event",string_of_Uint64(et.event->id));
		m.add("tournament",(et.event->tournament == NULL ? "" : string_of_Uint64(et.event->tournament->id)));
		m.add("time",string_of_int(et.time));
		cerr << "[Server::startEvents] PROCESSING " << m.dump() << endl;
		mod->process(instance->watcher,m);
		instance->events.pop_front();
		goto go_again;
	} 
}

void Server::alarmhandler(int s)
{
	instance->pong_flag = true;
}

void Server::addEvents(list<Event*>& el)
{
	EventTimer et;
	list<Event*>::iterator j;
	for (j = el.begin(); j != el.end(); ++j) {
		et.time = (*j)->date;
		et.event = *j;
		list<EventTimer>::iterator i;
		bool inserted = false;
		for (i = instance->events.begin(); i != instance->events.end(); ++i) {
			if (i->time > et.time) {
				instance->events.insert(i,et);
				inserted = true;
			}
		}
		if (! inserted) {
			instance->events.push_back(et);
		}
	}
	//instance->events.push_back(et);
}

void Server::removeEvent(Event* e)
{
	list<EventTimer>::iterator i;
	for (i = instance->events.begin(); i != instance->events.end(); ++i) {
		if (i->event == e) {
			EventTimer et = *i;
			instance->events.remove(et);
			return;
		}
	}
}

bool Server::saveEvents()
{
	bool retval = true;
	string evs;
	list<EventTimer>::iterator i;
	for (i = instance->events.begin(); i != instance->events.end(); ++i) {
		if (evs.empty()) {
			evs = string_of_int(i->time) + "|" + (i->event == NULL ? "" : string_of_Uint64(i->event->id));
		} else {
			evs += "," + string_of_int(i->time) + "|" + (i->event == NULL ? "" : string_of_Uint64(i->event->id));
		}
	}
	string event_update = "UPDATE events SET events = '" + evs + "' WHERE server = " + string_of_Uint64(cs()->id);
	Database* d = DBPool::grab();
	d->begin();

	Result* res = d->query(event_update);
	if ( res == NULL || ! res->success() || ! d->commit()) {
		d->rollback();
		cerr << "[Server::saveEvents] failed " << endl;
		retval = false;
	}
	
	delete res;
	DBPool::release(d);
	return retval;
}

bool Server::loadEvents()
{
	CS* c = cs();
	if (c == NULL) {
		cerr << "[Server::loadEvents] no such CS" << endl;
		return false;
	}
	
	bool retval = true;
	Database* d = DBPool::grab();
	d->begin();

	string event_load = "SELECT events.events FROM events WHERE server = " + string_of_Uint64(c->id);
	cerr << "[Server::loadEvents] " << event_load << endl;

	Result* res = d->query(event_load);
	if (res == NULL || ! res->success() || ! d->commit()) {
		cerr  << "[Server::loadEvents] failed " << endl;	
		d->rollback();
		retval = false;
	} else {
		if (res->rows == 0) goto cleanup;
		string r = (*res)["events"][0];
		if (r.empty()) goto cleanup;
		list<string> ets = split(',', r);
		list<string>::iterator i;
		for (i = ets.begin(); i != ets.end(); ++i) {
			cerr << "[Server::loadEvents] found EventTimer: " << *i << endl;
			list<string> fields = split('|',*i);
			EventTimer et;
			et.time = int_of_string(fields.front());
			fields.pop_front();
			cerr << "[Server::loadEvents] loading Event " << fields.front() << " at " << ctime(&et.time) << endl;
			et.event = Cache::find<Event>(fields.front());
			instance->events.push_back(et);
		}
	}
cleanup:
	delete res;
	DBPool::release(d);
	return retval;
}

bool Server::pendingEvents(Message& m)
{
	int j = 0;
	list<EventTimer>::iterator i;
	for ( i = instance->events.begin(); i != instance->events.end(); ++i) {
		++j;
		string key = string_of_int(j);
		string value = string_of_Uint64(i->event->id)
			+ "!" + i->event->name
			+ "!" + string_of_int(i->time);
		m.add(key,value);
	}	
	return j > 0;
}

CS* Server::cs()
{
	return instance->csptr;
}

Socket* Server::set_watcher(Socket* c)
{
	return instance->watcher = c;
}

Socket* Server::get_watcher()
{
	return instance->watcher;
}

list<Table*> Server::get_tables()
{
	list<Table*> retval;
	map<string,Room*>::iterator i;
	for (i = instance->rooms.begin(); i != instance->rooms.end(); ++i) {
		if ((i->second) == NULL) continue;
		Table* t = (i->second)->table();
		if (t == NULL) continue;
		retval.push_back(t);
	}
	return retval;
}

bool Server::myEvents(Player* p, Message& m)
{
	list<League*>::iterator l;
	list<EventTimer>::iterator i;
	for (i = instance->events.begin(); i != instance->events.end(); ++i) {
		if (i->event == NULL) continue;
		for (l = p->leagues.begin(); l != p->leagues.end(); ++l) {
			if (*l != i->event->season->league) continue;	
			m.add(string_of_Uint64(i->event->id), i->event->name + "!" + string_of_int(i->time) + "!" + i->event->game + "!" + (*l)->name  + "!" + (i->event->playoff ? "1" : "0") + "!" + i->event->notes);
		}
	}
	return true;
}

bool Server::send_watcher(Message& m)
{
	return instance->watcher->send(m);
}
