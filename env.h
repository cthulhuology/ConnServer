// env.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// environment variable handling

#ifndef __HAVE_ENV_H__
#define __HAVE_ENV_H__

#include <string>
#include <iostream>
#include <map>
#include <algorithm>
using namespace std;


class Env {
	public:
		string operator[](const string& s);
		void dump();
		static Env env;	
		Env();
		~Env();
	protected:
		Env(const Env&);	
		Env& operator=(const Env&);
	private:
		map<string,string> kv;
};

#endif
