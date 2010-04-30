// message.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//

#include "message.h"
#include "util.h"

bool Message::parse(const string& m)
{
	const char* err;
	vector<string> v;

	if(NULL != (err = find_key(m.c_str(),&v))) {
		cerr << "Message::parse error at: " << err << endl;
		return false;
	}
	data.clear();
	int i;
	for (i = 0; i < v.size(); i += 2) {
		data[v[i]] = v[i+1];
	}
	return true;
}

string Message::build()
{
	string r = "";
	map<string,string>::iterator i;
	for (i = data.begin();i != data.end();) {
		string k = (*i).first;
		string v = (*i).second;
		r +=  k + "=" + v;
		if (++i !=  data.end()) r += "&";
	}
	r += "~";
	return r;
}

string Message::dump()
{
	string r = "";
	if (data.size()) {
		map<string,string>::iterator i;
		for (i = data.begin(); i != data.end(); ++i) {
			if ( r.empty()) {
				r = i->first;
			} else {
				r += "&" + i->first;
			}
			r += "=";
			r += i->second;
		}
	} else {
		cerr << "No data in message to dump" << endl;
	}
	return r;
}

vector<string>* Message::keys()
{
	vector<string>* r = new vector<string>;
	map<string,string>::iterator i;

	for (i = data.begin(); i != data.end(); ++i) {
		r->push_back((*i).first);	
	}
	return r;
}

const string& Message::operator[](const char* k)
{
	return data[string(k)];
}

const string& Message::operator[](const string& k)
{
	return data[k];
}

void Message::add(const string& k, const string& v)
{
	data[k] = v;
}

void Message::add(const char* k, const char* v){
	data[string(k)] = string(v);
}

void Message::remove(const string& k)
{
	data.erase(k);
}

void Message::remove(const char* k)
{
	data.erase(k);
}

Message::Message()
{

}

Message::Message(const string& m)
{
	if (! parse(m.data())) {
		cerr << "Message::Message failed to initialize [" << m << "]" << endl;
	}
}

Message::~Message()
{

}

// private methods

string Message::encode(const string& s)
{
	string r;
	int i,pcount = 0;
	for (i=0; i<s.size(); i++) {
		if (isunreserved(s[i])) continue;
		pcount++;
	}

	r.reserve(s.size() + pcount*2);
	int j = 0;
	for (i=0; i<s.size(); i++) {
		if (isunreserved(s[i])) {
			r[j] = s[i];
		} else {
			r[j] = '%';
			char2hex(r,j+1,s[i]);
		}
		j++;
	}
	return r;
}

string Message::decode(const string& s)
{
	string r;
	int i,pcount = 0;
	for (i=0; i<s.size(); i++) {
		if (s[i] == '%') ++pcount;
	}
	if (pcount == 0) {
		r = s;
		return r; 
	}

	r.reserve(s.size() - pcount*2);
	int j = 0;
	for (i=0; i<s.size(); i++) {
		if (s[i] == '%') {
			r[j] = 16*hex2char(&(s.data())[i+1]) + hex2char(&(s.data())[i+2]);
			i += 2;
		} else {
			r[j] = s[i];
		}
		j++;
	}
	return r;
}

const char* Message::find_key(const char* m, vector<string>* v)
{
	const char* s = m;
	if (s ==  NULL || *s == '\n' || *s == '\0' || *s == '\r') return NULL;
	while ( s && *s != '\0' && *s != '=' && *s != '&' && *s != '\n' && *s != '\r') s++;
	if (*s == '=') { // We have a well formed key)
		v->push_back(string(m,(int)(s-m)));
		return find_value(s+1,v);
	}
	cerr << "Message:findkey invalid key=value pair, key without value [" << m << "]" << endl;
	return m;
}

const char* Message::find_value(const char* m, vector<string>* v)
{
	const char* s = m;
	if (s ==  NULL || *s == '\0' || *s == '\n' || *s == '\r') {
		v->push_back("");
		return NULL;
	}
	while ( s && *s != '\0' && *s != '=' && *s != '&' && *s != '\n' && *s != '\r') s++;
	if (*s == '&' || *s == '\0' || *s == '\n' || *s == '\r') {
		v->push_back(string(m,(int)(s-m)));
		if (*s != '\0' && *s != '\n' && *s != '\r') return find_key(s+1,v);
		return NULL;
	}
	cerr << "Message::find_value Invalid key=value pair, empty value, key=key [" << m << "]" << endl;
	return m;
}

char Message::hex2char(const char* s)
{
	if (*s >= '0' && *s <= '9') return *s - '0';
	if (*s >= 'a' && *s <= 'f') return *s - 'a' + 10;
	if (*s >= 'A' && *s <= 'F') return *s - 'A' + 10;
	cerr << "Inavlid character '" << *s << "'" << endl;
	return '\0';
}

void Message::char2hex(string& s, int pos, char c)
{
	char a, b;
	a = c / 16;
	b = c % 16;
	s[pos] = a < 10 ? a + '0' : a - 10 + 'a';
	s[pos+1] = b < 10 ? b + '0' : b - 10 + 'a';
}

bool Message::isunreserved(char c)
{
	return (c >= 'a' && c <= 'z') ||
	(c >= 'A' && c <= 'Z') ||
	(c >= 'O' && c <= '9') ||
	c == '-' || c == '.' || c == '_' || c == '~';
}

bool Message::check(const string& params)
{
	list<string>::iterator i;
	list<string> pl = split(',',params);
	for (i = pl.begin(); i != pl.end(); ++i) {
		if ( (*this)[*i].empty() ) {
			cerr << "[Message::check] Message " <<  (*this)["msg"] << " missing parameter " << *i << endl;
			(*this).add("status","1");
			return false;
		}
	}
	return true;
}

