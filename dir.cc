// dir.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
//	Directory handling routines
//

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dir.h"

Directory::Directory(const string& p)
	: path(p)
{
	dir = opendir(p.c_str());	
	if (dir == NULL) {
		cerr << "Directory::Directory failed to open directory " << path << endl;
		return;
	}
	
	struct dirent* de;

	while (NULL != (de = readdir(dir))) {
		if (de->d_type == DT_DIR) {
			drs.push_back(string(de->d_name));	
		} else {
			fds[string(de->d_name)] = -1;
		}
	}
}

Directory::~Directory()
{
}

int Directory::operator[](const string& fname)
{
	if (fds[fname] == -1 || ! fds[fname]) {
		fds[fname] = open((path + "/"  + fname).c_str(),O_RDWR|O_CREAT,0600);
	} 
	return fds[fname];
}

Directory* Directory::subdir(const string& dirname)
{
	bool found = false;
	vector<string>::iterator i;
	for (i = drs.begin(); i != drs.end(); ++i) {
		if ( *i == dirname ) {
			found = true;
			break;
		}
	}	
	if(! found) {
		if (mkdir((path + "/" + dirname).c_str(),0700)) {
			cerr << "Directory[] could not create " <<
				path << "/" << dirname << endl;
			return NULL;
		}
	}
	return new Directory(path + "/" + dirname);
}

vector<string> Directory::files()
{
	vector<string>* retval = new vector<string>;
	map<string,int>::iterator j;
	for (j = fds.begin(); j != fds.end(); ++j) {
		retval->push_back(path + "/" + (*j).first);
	}
	return *retval;
}

vector<string> Directory::dirs()
{
	return drs;
}

void Directory::dump()
{
	cerr << "Path: " << path << endl;
	cerr << "Directories:" << endl;
	vector<string>::iterator i;
	for (i = drs.begin(); i != drs.end(); ++i) {
		cerr << "\t" << *i << endl;
	}
	cerr << "Files:" << endl;
	map<string,int>::iterator j;
	for (j = fds.begin(); j != fds.end(); ++j) {
		cerr << "\t" << (*j).first << endl;
	}
}

bool Directory::exists(const string& fname)
{
	map<string,int>::iterator j;
	for (j = fds.begin(); j != fds.end(); ++j)  {
		if (j->first == fname) {
			return true;
		}
	}
	return false;
}

#ifdef TEST_CASE
int main (int argc, char** argv)
{
	Directory d(".");
	d.dump();
	return 0;
}
#endif
