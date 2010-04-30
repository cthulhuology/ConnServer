// dir.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
//	Directory handling stuff

#ifndef __HAVE_DIRECTORY__H__
#define __HAVE_DIRECTORY__H__

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <dirent.h>
using namespace std;

class Directory {
	public:
		Directory(const string& p);
		~Directory();
		bool exists(const string& fname);
		int operator[](const string& fname);
		Directory* subdir(const string& dirname);
		vector<string> files();
		vector<string> dirs();
		void dump();
	private:
		string path;
		map<string,int> fds;
		vector<string> drs;
		DIR* dir;
};

#endif
