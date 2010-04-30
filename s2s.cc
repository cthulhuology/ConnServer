// s2s.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// Implementation of server to server communications
// I reiterate this is not intended to provide message
// forwarding, this only meant to handle coordination
// and load balancing.

ServerMap* ServerMap::servermap = NULL;
bool ServerMap::initialized = false;

ServerMap* ServerMap::init(const string& s)
{
	if (initialized) {
		cerr << "ServerMap::init already initialized" << endl;
		return servermap;
	}
	servermap = new ServerMap(s);
	return servermap;
}

bool ServerMap::finalize(ServerMap* sm)
{
	if (!initialized || sm == NULL) return false;
	map<string,Socket*>::iterator i;
	for(i = sm->svrs.begin(); i != sm->svrs.end(); ++i) {
		Socket::finalize((*i).second);
	}
	delete sm;
	return true;
}

bool ServerMap::add(const string& h, const string& p)
{
	m.add("msg","s2sinit");
	m.add("to",s);
	m.add("from",name);
	
	Socket* s = Socket::create();
	if (! s->conn(s,p)) {
		cerr << "ServerMap::add failed to connect: " << h << ":" << p << endl;
		return false;
	}

	s->out(m.build());

	svrs[h] = s;	
	return true;
}

bool SeverMap::remove(const string& s)
{
	Socket* k = svrs[s];
	if (! k) {
		cerr << "SeverMap::remove no such server " << s << endl;
		return false;
	}
	Socket::finalize(k);
	svrs.erase(s);	
	return true;
}

Socket* ServerMap::find(const string& s)
{
	Socket* k = svrs[s];
	if (! k) {
		cerr << "ServerMap::find failed to find server " << s << endl;
		return NULL;
	}
	return k;
}

bool send(const string& s, Message& m) 
{
	string ms = m.build();
	Socket* k = svrs[s];
	if (!k || ! k->connected()) {
		cerr << "ServerMap::send failed to find server " << s << endl;
		return false;
	}
	if (ms.size() > k->out(ms)) {
		return false;
	}
	return true;
}

void split_host_port(const string& s, string& h, string& p)
{
	size_t i = s.find(":",0);
	if (i == npos) {
		h = s;
		p = "9999";
		return;
	}
	h = s.substr(0,i-1);
	p = s.substr(i+1,npos);
	return;
}

bool discover(vector<string>& v)
{
	vector<string>::iterator i;
	for (i = v.begin(); i != v.end(); ++i) {
		string h,p;
		split_host_port(*i,h,p);
		if (NULL == find(h) && add(h,p)) { // only discover on servers we don't already have mapped.
			Message m;
			cerr << "Contacted " << *i << endl;
			m.add("msg","s2sdiscover");
			m.add("to",h);
			if (! send(h,m)) {
				cerr << "ServerMap::discover failed to contact " << h << endl;
			}
		} else {
			cerr << "ServerMap::discover failed to discover " << h << ":" << p << endl;
		}
	}
}


ServerMap::ServerMap(const string& s)
	: name(s)
{
}
