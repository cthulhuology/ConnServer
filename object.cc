// data.cc
//
//	Copyright (C) 2005,2010 David J. Goehrig
//	All Rights Reserved.
//
// object wrapper classes
//

#include "object.h"
#include "cache.h"

// Object Class

Object::Object() { }
Object::~Object() { }

bool Object::init(const string& s) { 
	cerr << "[Object::init] " << s << endl;
	return true; 
}
string Object::tos() { return string(""); }

long long Object::create(Database* d, const string& t) {
	long long retval = 0LL;
	string query = "SELECT * FROM new_object(" + string_of_Uint64(Cache::next()) + ",'" + t + "','')";
	cerr << "[Object::create] " << query << endl;
	Result* res = d->query(query);
	if (res == NULL || ! res->success() || res->rows != 1) {
		cerr << "[Object::create] Failed to allocate storage: " << d->error() << endl;
	} else { 
		retval = Uint64_of_string((*res)["new_object"][0]);
	}
	delete res;
	return retval;	
}

string escape_tick(string& retval, char c) {
	if (c == '\'') return retval + "%27";
	return retval + c;
}

string escaped(const string& s) {
	string retval = "";
	for (int i = 0; i < s.size(); ++i) retval = escape_tick(retval,s[i]);
	return retval;
}

bool Object::save(Database* d) {
	bool retval = true;
	string query = "SELECT * FROM save_object(" + string_of_Uint64(id) + ",'" + escaped(tos()) + "')";
	cerr << "[Object::save] " << query << endl;
	Result* res = d->query(query);	
	if (res == NULL || ! res->success()) {
		cerr << "[Object::save] Failed to save object " << id << " because: " << d->error() << endl;
		retval = false;
	}
	delete res;
	return retval;
}

bool Object::load() {
	bool retval = true;
	string query = "SELECT * FROM load_object(" + string_of_Uint64(id) + ")";
	cerr << "[Object::load] " << query << endl;
	Database* d = DBPool::grab();
	cerr << "[Object::load] Database is " << d << endl;
	d->begin();
	Result* res = d->query(query);
	cerr << "[Object::load] Result is " << res << endl;
	d->commit();
	DBPool::release(d);
	cerr << "[Object::load] Result is " << (res->success() ? "Success!" : "Failure!") << endl;
	if (res == NULL || ! res->success()) {
		cerr << "[Object::load] Failed to load object " << id << " because: " << d->error() << endl;
		retval = false;
	} else if ( res->rows == 0 ) {
		cerr << "[Object::load] Object " << id << " not found" << endl;
		retval = false;
	} else if ( res->rows > 1) {
		cerr << "[Object::load] Waring multiple objects " << id << " database corrupted" << endl;
		retval = false;
	} else {
		cerr << "[Object::load] Init with value: " << (*res)["load_object"][0]  << endl;
		retval = init((*res)["load_object"][0]);
	}
	delete res;
	cerr << "[Object::load] released" << endl;
	return retval;
}

const string Object::type() {
	return string(typeid(*this).name());
}

map<string,string> Object::parse(const string& s) {
	map<string,string> retval;
	list<string> kv_pairs = split('\n',s);
	list<string>::iterator i;
	for (i = kv_pairs.begin(); i != kv_pairs.end(); ++i) {
		cerr << "--- " << *i << endl;
		list<string> kv = split(':',*i);
		if (kv.size() >= 2 ) {
			string first = kv.front();
			kv.pop_front();
			string second = kv.front();
			retval[first] = second;
			cerr << "KEY=VALUE: " << first << "=" << second << endl;
		} else if (kv.size() == 1) {
			retval[kv.front()] = "";
		}
	}
	return retval;
}




