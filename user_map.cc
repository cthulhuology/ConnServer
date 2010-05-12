// user_map.cc
//
// © 2004, 2005, 2010 David J. Goehrig
//
// Mapping users to clients and sockets and player ids so that we can track them
//

#include "user_map.h"
#include "room.h"
#include "socket.h"
#include "util.h"
#include "object.h"
#include "login.h"

UserMap* UserMap::usermap = NULL;

UserMap::UserMap()
{
}

UserMap::~UserMap()
{
}

bool 
UserMap::init()
{
	if (usermap != NULL) {
		cerr << "UserMap::init already initialized" << endl;
		return UserMap::usermap;
	}
	usermap = new UserMap();
	return usermap != NULL;
}

bool 
UserMap::finalize()
{
	if (usermap == NULL) return false;
	delete usermap;
	usermap = NULL;
	return true;
}

bool 
UserMap::add(Login* l)
{
	if (usermap == NULL) return false;
	if (l == NULL || l->sock == NULL) return false;
	if (null() != find(l->sock)) {
		cerr << "UserMap::add socket already added" << endl;
		return false;
	}
	usermap->logins.push_back(l);
}

bool 
UserMap::remove(Socket* s)
{
	list<Login*>::iterator ue = find(s);
	if (ue == null()) {
		cerr << "UserMap::remove failed for socket " << s << endl;
		return false;
	}
	usermap->logins.erase(ue);
	return true;
}

list<Login*>::iterator 
UserMap::null()
{
	return usermap->logins.end();
}

list<Login*>::iterator 
UserMap::find(const Socket* s)
{
	if (usermap == NULL) {
		cerr << "[UserMap::find failure, no usermap]" << endl;
		exit(1);
	}
	list<Login*>::iterator i;
	for (i = usermap->logins.begin(); i != usermap->logins.end(); ++i) {
		if ((*i)->sock == s) return i;
	}
	return null();
}

list<Login*>::iterator 
UserMap::find(const string& u)
{
	if (usermap == NULL) {
		cerr << "[UserMap::find failure, no usermap]" << endl;
		exit(1);
	}
	list<Login*>::iterator i;
	for (i = usermap->logins.begin(); i != usermap->logins.end(); ++i) {
		if (string_of_Uint64((*i)->player->id) == u) {
			return i;
		}
	}
	return null();
}

list<Socket*> 
UserMap::sockets()
{
	list<Socket*> retval;
	list<Login*>::iterator i;
	for (i = usermap->logins.begin(); i != usermap->logins.end(); ++i) {
		retval.push_back((*i)->sock);
	}
	return retval;
}

string
UserMap::users()
{
	string retval;
	list<Login*>::iterator i;
	for (i = usermap->logins.begin(); i != usermap->logins.end(); ++i) {
		if (retval.empty()) {
			retval = (*i)->player->id;
		} else {
			retval += "!" + (*i)->player->id;
		}
	}
	return retval;
}

Login*
UserMap::getLogin(const string& u)
{
	list<Login*>::iterator i = find(u);
	if (i == null()) {
		cerr << "UserMap::getLogin failed to find " << u << endl;
		return NULL;
	}
	return *i;
}

Login*
UserMap::getLogin(const Socket* s)
{
	list<Login*>::iterator i = find(s);
	if (i == null()) {
		cerr << "UserMap::getLogin failed to find " << s << endl;
		return NULL;
	}
	return *i;
}

