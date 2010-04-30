// esocket.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
//

#include "esocket.h"
#include "pool.h"
#include "server.h"
#include "room.h"
#include "message.h"
#include "module.h"
#include "user_map.h"

Socket* Socket::create()
{
	Socket* r;
	int fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (fd == -1) {
		cerr << "Socket::create failed to create socket" << endl;
		return NULL;
	}
	r = new Socket(fd,NULL);
	
	Pool::add(r);

	return r;
}

Socket* Socket::open()
{
	Socket* r;
	struct sockaddr_in naddr;
	int nfd, naddrlen = sizeof(addr);
	while (0 > (nfd = accept(fd,(sockaddr*)&naddr,(socklen_t*)&naddrlen))) {
		if (errno == EINTR)  {
			cerr << "Socket::open accept failed\n" << endl;
			continue;
		}
		if (errno != EAGAIN && errno != EWOULDBLOCK)  {
			cerr << "Socket::open accept failed\n" << endl;
			return NULL;
		}
	}

	r = new Socket(nfd,&naddr);
	
	Pool::add(r);

	return r;
}

string Socket::in()
{
	int l;
	char r;
	string s = "";
read_again:
	l = read(fd,&r,1);
	cerr << "[Socket::in] read " << l << endl;
	if (l < 0) {
		if (errno == EAGAIN) {
			goto read_done;
		}
		cerr << "[Socket::in] IO Error on fd " << fd <<  " for Socket " << this << endl;
		conned = false;
		return s;
	}
	if (l > 0)  {
		if (r != '~' && r !='\r' && r!= '\n') s += r;
		else s += '\0';
		goto read_again;
	}
	cerr << "[Socket::in] disconnnected on fd " << fd << " for Socket " << this <<  endl;
	conned = false;
read_done:
	return s;
}

int Socket::out(const string& s)
{
	size_t written = 0;
	size_t len = s.size();
	while (written < len) {
		size_t t = write(fd,s.data() + written, len - written);
		if (t < 0) {
			if (errno == EAGAIN) continue;
			if (errno == EPIPE) {
				cerr << "[Socket::out] EPIPE error disconnected" << endl;
				conned = false;
			}
			cerr << "Socket::out IO error" << endl;
			return written;
		}
		written += t;
	}
	write(fd,"\0",1);
	return written;
}

// Socket::monitor

bool Socket::monitor(int port)
{
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	uint16_t p = port;
	addr.sin_port = htons(p);	

	int tmp = 1;
	if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&tmp,sizeof(int))) {
		cerr << "Socket::monitor Failed to set SO_REUSEADDR" << endl;
		return false;
	}

	struct linger lin;
	lin.l_onoff = 1;
	lin.l_linger = 20;
	if (setsockopt(fd,SOL_SOCKET,SO_LINGER,&lin,sizeof(lin))) {
		cerr << "Socket::monitor Failed to set SO_LINGER" << endl;
		return false;
	}

	if (0 > bind(fd,(struct sockaddr*)&addr,sizeof(addr))) {
		cerr << "Socket::monitor Failed to bind to " << port << endl;
		return false;
	}

	if (0 > listen(fd,MAX_CONNS)) {
		cerr << "Socket::monitor Faild to listen on " << port << endl;
		return false;
	}
	return true;
}

bool Socket::conn(const char* host, const char* port)
{
	struct hostent* hi;
	uint16_t p = atoi(port);

	hi = gethostbyname(host);
	if (hi == NULL) {
		cerr << "Can not find hostname " << host << endl;
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(p);
	addr.sin_addr = *(struct in_addr *) hi->h_addr;
	
	if ( 0 > connect(fd,(struct sockaddr*)&addr,sizeof(addr))) {
		if (errno == EINPROGRESS) return true;
		cerr << "Failed to connect to host " << host << ":" << port << endl;
		perror("connect");
		return false;
	}
	return true;
}

void Socket::finalize(Socket* s)
{
	if (s->rm != NULL) {
		s->rm->remove(s);
	}
	delete s;	
}

int Socket::descriptor()
{
	return fd;
}

bool Socket::connected()
{
	cerr << "[Socket::connected] conned? " << (conned ? "yes" : "no" ) << endl;
	return conned;
}

void Socket::disconnect()
{
	Message msg;
	Module* mod = NULL;
	cerr << "[Socket::disconnect] socket " << this << endl;
	if (isBot()) {
		cerr << "[Socket::disconnect] disconnecting bot" << endl;
		Module::call("bot-disconnect",this,msg);
		if (this == Server::get_watcher()) {
			cerr << "[Socket::disconnect] setting watcher to null" << endl;
			Server::set_watcher(NULL);	
		}
	} else {
		cerr << "[Socket::disconnect] disconnecting player" << endl;
		Module::call("disconnect",this,msg);
	}
	if (log != NULL) {
		UserMap::remove(this);
	}
	Socket::finalize(this);		
}

bool Socket::send(Message& m)
{
	if (! connected()) return false;
	string s = m.build();
//	cerr << "[Socket::send] " << s << endl;
	if (s.size() > out(s)) {
		cerr << "[Socket::send failed " << s << "]" << endl;
		return false;
	} 
	return true;
}

bool Socket::read_msg(Message& m, string& s)
{
	if (s.empty()) {
		s = in();
	} else {
		s += in();
	}
//	cerr << "[Socket::read_msg] '" << s <<  "' size: " << s.size() << (s.empty() ? " empty " : " not empty ") << endl;
	if (s.empty()) {
	//	cerr << "[Socket::read_msg empty " << this << " ]" << endl;
		return false;
	}
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] == '\0') {
//			cerr << "[Socket::read_msg parsing]" << endl;
			m.parse(s.substr(0,i));	
//			cerr << "[Socket::read_msg dump] " << m.dump() << endl;
			if ((i+2) < s.size()) {
				 s = s.substr(i+1);
//				cerr << "new s '" << s << "' size:" <<  s.size() <<  endl;
			} else {
//				cerr << "[Socket::read_msg clearing s]" << endl;
				 s = "";
			}
			return true;
		}
	}
//	cerr << "[Socket::read_msg no null]" << endl;
	return true;
}

Room* Socket::room()
{
	if (rm == NULL) return Server::room("lobby");
	return rm;
}

Login* Socket::login()
{
	return log;
}

void Socket::set_login(Login* l)
{
	log = l;
}

void Socket::set_room(Room* r)
{
	rm = r;
}


bool Socket::isBot()
{
	if (rm == NULL) return false;
	return rm->isBot(this);	
}

Socket::Socket(int f, struct sockaddr_in* ad)
	: fd(f)
{
	log = NULL;
	rm = NULL;
	conned = true;
	cerr << "[Socket::Socket] socket " << this << " conned = " << (conned ? "yes" : "no") << endl;
	if (ad != NULL) {
		memcpy(&addr,ad,sizeof(struct sockaddr_in));
	}
	nonblock(fd);
}

Socket::~Socket()
{
	Pool::remove(this);
	close(fd);
}

void Socket::nonblock(int fd)
{
	int of = fcntl(fd,F_GETFL,0);
	if (of == -1) {
		cerr << "Socket::nonblock failed" << endl;
		return;
	}
	if (fcntl(fd,F_SETFL,of|O_NONBLOCK)) {
		cerr << "Socket::nonblock failed" << endl;	
	}
}
