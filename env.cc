// env.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// environment variable handling

#include "env.h"

extern char** environ;

Env Env::env;

Env::Env()
{
	char** i;
	size_t o;
	string tmp;
	for (i = environ; *i != NULL; ++i) {
		tmp = *i;
		o = tmp.find('=');
		if (o == (size_t)-1) {
			kv[tmp] = "true";
		} else {
			kv[tmp.substr(0,o)] = tmp.substr(o+1,(size_t)-1);
		}
	}
}

Env::~Env()
{
}

void Env::dump()
{
	map<string,string>::iterator i;

	for (i = kv.begin(); i != kv.end(); ++i) {
		cerr << (*i).first << " = " << (*i).second << endl;
	}
}

string Env::operator[](const string& s)
{
	return kv[s];
}


#ifdef TEST_CASE
int 
main(int argc, char** argv) 
{
	env.dump();
	return 0;
}
#endif
