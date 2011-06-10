// socket.h
//
// Copyright (C) 2004, 2005, 2010 David J. Goehrig
//

#ifndef __HAVE_SOCKET_H__
#define __HAVE_SOCKET_H__

#include <vector>
#include <map>
#include <iostream>

// USE_GNU for linux specific features of fcntl
#ifndef __USE_GNU
#define __USE_GNU
#include <fcntl.h>
#undef __USE_GNU
#else
#include <fcntl.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

class Room;
class Message;
class Login;

class Socket {
	public:
		static Socket* create();	// create a new connection
		Socket* open();			// accept a new connection
		string in();
		int out(const string& s);
		bool monitor(int port);
		bool conn(const char* host, const char* port);
		static void finalize(Socket* s);
		int descriptor();
		bool connected();
		void disconnect();
		bool send(Message& m);
		bool read_msg(Message& m, string& s);
		Login* login();
		void set_login(Login* l);
		Room* room();
		void set_room(Room* r);
		bool isBot();
	protected:
		Socket();
		Socket(int f, struct sockaddr_in* ad);
		Socket(const Socket&);
		Socket& operator=(const Socket&);
		~Socket();
	private:
		static const int MAX_CONNS = 30;
		static const int BUFFER_SIZE = 1024;
		static void nonblock(int fd);
		pthread_mutex_t mut;
		bool conned;
		int fd;
		struct sockaddr_in addr;
		Login* log;
		Room* rm;
};

#endif
