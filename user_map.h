// user_map.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// The userMap object maintains a mapping between uid & Client* & Socket*
// It is used for tracking connections. (this replaces the users & children Server stuff)

#ifndef __HAVE_USER_MAP_H__
#define __HAVE_USER_MAP_H__

#include <list>
#include <string>
#include <vector>

using namespace std;

class Socket;
class Login;

class UserMap {
	public:
		static bool init();
		static bool finalize();
		static bool add(Login* l);
		static bool remove(Socket *s);
		static Login* getLogin(const string& u);
		static Login* getLogin(const Socket* s);
		static list<Socket*> sockets();
		static string users();
	protected:
		UserMap();
		UserMap(const UserMap&);
		~UserMap();
		UserMap& operator=(const UserMap&);
	private:
		static UserMap* usermap;
		list<Login*> logins;
		static list<Login*>::iterator find(const Socket* s);
		static list<Login*>::iterator find(const string& u);
		static list<Login*>::iterator null();
};

#endif
