// message.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// Handles parsing and printing of messages
//

#ifndef __HAVE_MESSAGE__
#define __HAVE_MESSAGE__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <iostream>

#define MSG_CHECK(x) if (! m.check( x )) return c->send(m);

using namespace std;

class Message {
	public:
		bool parse(const string& m);	
		string build();
		string dump();
		vector<string>* keys();
		const string& operator[](const char* k);
		const string& operator[](const string& k);
		void add(const string& k, const string& v);
		void add(const char* k, const char* v);
		void remove(const string& k);
		void remove(const char* k);
		bool check(const string& params);
		Message();
		Message(const string& m);
		~Message();
	private:
		map<string,string> data;
		string encode(const string& s);
		string decode(const string& s);
		const char* find_key(const char* m, vector<string>* v);
		const char* find_value(const char* m, vector<string>* v);
		char hex2char(const char* s);
		void char2hex(string& s, int pos, char c);
		bool isunreserved(char c);

};

#endif
