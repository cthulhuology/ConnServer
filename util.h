// util.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//

#ifndef __HAS_UTIL__
#define __HAS_UTIL__

#include <string>
#include <list>
#include <stdio.h>

using namespace std;

bool isnum(const string& s);

string string_of_int(int i);
string string_of_Uint64(long long i);

int int_of_string(const string& s);
long long Uint64_of_string(const string& s);

string times_ten(const string& s);
string divides_ten(const string& s);

list<string> split(char d, const string& s);
string join(char d, list<string>& v);

list<int> int_list_of_string_list(list<string>& l);
list<string> string_list_of_int_list(list<int>& l);

string hexdecode(const string& s);
string hexencode(const string& s);

#endif
