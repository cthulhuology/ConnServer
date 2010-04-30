// pool.cc
//
//	Copyright (C) 2005 David J. Goehrig
//
//	A socket pooling system, for watching active sockets, supports both epoll and kqueue
//

// These headers a required to get the epoll stuff to work
#ifdef LINUX
#include <sys/epoll.h>
#endif

#include "pool.h"
#include <iostream>
using namespace std;

Pool* Pool::pool = NULL;

bool Pool::init(int max)
{
	if (pool != NULL) {
		cerr << "Pool::init already initialized " << pool->maxfds  << " connections" << endl;
		return false;
	}
	pool = new Pool(max);
	return true;
}

bool Pool::finalize()
{
	delete pool;
	pool = NULL;
	return true;
}

bool Pool::add(Socket* s, bool et)
{
	if ( pool->nevents >= pool->maxfds) {
		cerr << "Pool::add Failed, maximum descriptors added" << endl;
		return false;
	}
#ifdef LINUX
	struct epoll_event evn;
	if (et) {
		evn.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
	} else {
		evn.events = EPOLLIN | EPOLLERR | EPOLLHUP; 
	}
	evn.data.fd = s->descriptor();
	if (0 > epoll_ctl(pool->poolfd,EPOLL_CTL_ADD,s->descriptor(),&evn)) {
		cerr << "Pool::add Failed to add descriptor " << s->descriptor() << endl;
		return false;
	}
#else 
	struct kevent evn;
	EV_SET(&evn,s->descriptor(),EVFILT_READ,EV_ADD,0,0,0);
	pool->change_list.push_back(evn);
#endif
	pool->nevents++;
	pool->fds[s->descriptor()] = s;
	return true;
}

bool Pool::remove(Socket* s, bool et)
{
#ifdef LINUX
	struct epoll_event evn;
	if (et) {
		evn.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
	} else {
		evn.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	}
	evn.data.fd = s->descriptor();
	if (0 > epoll_ctl(pool->poolfd,EPOLL_CTL_DEL,s->descriptor(),&evn)) {
		cerr << "Pool::add Failed to add descriptor " << s->descriptor() << endl;
		return false;
	}
#else
	struct kevent evn;
	EV_SET(&evn,s->descriptor(),EVFILT_READ,EV_DELETE,0,0,0);
#endif
	pool->nevents--;
	pool->fds.erase(s->descriptor());
	return true;
}

Socket* Pool::find(int fd) 
{
	Socket* r = (*pool)[fd];
	if (! r) {
		cerr << "Pool::find no such fd " << fd << endl;
		return NULL;
	}
	return r;
}

int Pool::wait()
{
	int i,rdy;
	pool->rdyfds.clear();

again_again:
#ifdef LINUX
	rdy = epoll_wait(pool->poolfd,pool->events,pool->maxfds,TIMEOUT * 1000);
//	rdy = epoll_wait(pool->poolfd,pool->events,pool->maxfds,-1);
#else
	struct kevent* chnglst;
	chnglst = (pool->change_list.size() == 0 ? NULL : new struct kevent[pool->change_list.size()]);

	vector<struct kevent>::iterator j;
	i = 0;
	for (j = pool->change_list.begin(); j != pool->change_list.end(); ++j) {
		memcpy(&chnglst[i],&*j,sizeof(struct kevent));
		++i;
	}
	rdy = kevent(pool->poolfd,chnglst,i,pool->events,pool->nevents,&pool->timeout);
//	rdy = kevent(pool->poolfd,chnglst,i,pool->events,pool->nevents,NULL);
	delete[] chnglst;
	pool->change_list.clear();
#endif
	if (rdy < 0) {
		goto again_again;
	}

	for (i = 0; i < rdy; i++) {
#ifdef LINUX
		pool->rdyfds.push_back(pool->fds[pool->events[i].data.fd]);
#else 
		if (pool->events[i].filter == EVFILT_READ) {
			pool->rdyfds.push_back(pool->fds[pool->events[i].ident]);
		}
#endif
	}
	return pool->rdyfds.size();
}

int Pool::flush()
{
	int retval = 0;
	map<int,Socket*>::iterator i;
	vector<Socket*> v;
	for (i = pool->fds.begin(); i != pool->fds.end(); ++i) {
		if (i->second == NULL) continue;
		if (i->second->connected()) continue;
		v.push_back(i->second);
	}
	vector<Socket*>::iterator vi;
	for (vi = v.begin(); vi != v.end(); ++vi) {
		(*vi)->disconnect();
		++retval;
	}
	return retval;	
}

Socket* Pool::operator[](int i)
{
	if (i >= 0 && i < rdyfds.size()) {
		return rdyfds[i];
	}
	return NULL;
}

Pool::Pool(int max)
	: maxfds(max), nevents(0)
{
#ifdef LINUX
	poolfd = epoll_create(max);
	events = new struct epoll_event[max];
#else
	poolfd = kqueue();
	events = new struct kevent[max];
	timeout.tv_sec = TIMEOUT;
	timeout.tv_nsec = 0;
#endif
}

Pool::~Pool()
{
	delete[] events;
	close(poolfd);
}
