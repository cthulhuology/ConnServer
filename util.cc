// util.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//

#include "util.h"
#include <iostream>

using namespace std;

bool 
isnum(const string& s)
{
	bool retval = true;
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] >= '0' && s[i] <= '9') continue;
		retval = false;
		break;
	}
	return retval;
}

string
string_of_int(int i)
{
	char* buf = new char[32];
	int len = sprintf(buf,"%d",i);
	string s(buf,len);
	return s;
}

string 
string_of_Uint64(long long i)
{
	char* buf = new char[32];
	int len = sprintf(buf,"%llu",i);
	return string(buf,len);
}

int 
int_of_string(const string& s)
{
	int r = 0, i;
	if (s.empty()) return 0;
	for (i = 0; i < s.size(); ++i) {
		r = (r*10) + (s[i] - '0');	
	}
	return r;
}

long long 
Uint64_of_string(const string& s)
{
	long long r = 0;
	size_t i;
	if (s.empty()) return 0LL;
	for (i = 0; i < s.size(); ++i) {
		r = (r*10LL) + (long long)(s[i] - '0');	
	}
	return r;
}

string 
times_ten(const string& s)
{
	string r(s);
	r += "0";
	return r;
}

string
divides_ten(const string& s)
{
	string r(s,0,s.size()-1);
	if (r.empty()) r = "0";
	return r;
}

list<string> 
split(char d, const string& s)
{
	list<string> retval;
	size_t i,o;
	o = 0;
	for (i = 0; i < s.size(); ++i) {
		if (s[i] != d) continue;
		string e = s.substr(o,i-o);
	//	cerr << "e = " << e << endl;
		retval.push_back(e);
		o = (i+1);
	}
	if (i > o) {
		string e = s.substr(o,i-o);
	//	cerr << "e = " << e << endl;
		retval.push_back(e);
	}
	return retval;
}

string
join(char d, list<string>& v)
{
	string retval;
	list<string>::iterator i;
	for (i = v.begin(); i != v.end(); ++i) {
		if (retval.empty()) {
			retval = *i;
		} else {
			retval += d + *i;
		}
	}
	return retval;
}

list<int>
int_list_of_string_list(list<string>& l)
{
	list<int> retval;
	list<string>::iterator i;
	for (i = l.begin(); i != l.end(); ++i) {
		retval.push_back(int_of_string(*i));
	}
	return retval;
}

list<string>
string_list_of_int_list(list<int>& l)
{
	list<string> retval;
	list<int>::iterator i;
	for (i = l.begin(); i != l.end(); ++i) {
		retval.push_back(string_of_int(*i));
	}
	return retval;
}

char
dehex(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 0;
}

string
hexdecode(const string& s)
{
	string retval;
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] == '%') {
			retval += (dehex(s[i+1])*16 + dehex(s[i+2]));
			i += 2;
		} else {
			retval += s[i];
		}
	}
	return retval;

}

string
hexify(char c)
{
	string retval;
	char a = c / 16;
	char b = c % 16;
	retval = (a < 10 ? '0' + a : 'a' + (a - 10));
	retval += (b < 10 ? '0' + b : 'a' + (b - 10));
	return retval;
}

string
hexencode(const string& s)
{
	string retval;
	for (int i = 0; i < s.size(); ++i) {
		if (s[i] >= 'a' && s[i] <= 'z') {
			retval += s[i];
		} else if (s[i] >= 'A' && s[i] <= 'Z') {
			retval += s[i];
		} else if (s[i] >= '0' && s[i] <= '9') {
			retval += s[i];
		} else {
			retval += '%' + hexify(s[i]);
		}
	}
	return retval;
}
