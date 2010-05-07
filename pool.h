// pool.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved

#ifndef __HAVE_POLL_H__
#define __HAVE_POLL_H__

#include <vector>
#include <map>

#ifndef LINUX
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

#include "socket.h"

using namespace std;

class Socket;

class Pool {
	public:
		static bool init(int max);
		static bool finalize();
		static bool add(Socket* s, bool et=true);
		static bool remove(Socket* s, bool et=true);
		static Socket* find(int fd);
		static int wait();
		static int flush();
	protected:
		Pool();
		Pool(int max);
		Pool(const Pool&);
		~Pool();
		Pool& operator=(const Pool&);
	private:
		static Pool* pool;
		Socket* operator[](int i);
		const static int TIMEOUT = 10;
#ifdef LINUX
		struct epoll_event* events;
#else
		struct kevent* events;
		vector<struct kevent> change_list;
		struct timespec timeout;
#endif
		vector<Socket*> rdyfds;
		map<int,Socket*> fds;
		int poolfd;
		int maxfds;
		int nevents;
};

#endif
