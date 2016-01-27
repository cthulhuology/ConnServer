// Object Class
//
// © 2010 David J. Goehrig
// 

#ifndef __HAVE_OBJECT_H__
#define __HAVE_OBJECT_H__

#include <typeinfo>

#include <string>
#include <list>
#include <map>
#include <iostream>
#include <algorithm>
using namespace std;

#include "database.h"
#include "util.h"
#include "message.h"
#include "user_map.h"
#include "socket.h"


#define PROPERTY(x) x = o[#x];
#define INT_PROPERTY(x) x = int_of_string(o[#x]);
#define OBJ_PROPERTY(x,y) x = Cache::find<y>(o[#x]);
#define LIST_PROPERTY(x,y) x = decode<y>(o[#x]);

#define COMMON_METHODS(x) \
		static x* alloc(Database* d); \
		bool init(const string& s); \
		string tos(); \
		static x* cache_alloc(long long objid); \
		x(); \
		x(const x&);\
		~x(); \
		x& operator=(const x&); \
	friend class Cache;

#define ALLOC_METHODS(x) \
	x::x() {} \
	x::~x() {} \
	x* x::alloc(Database* d) { \
		x* retval = new x; \
		if (retval == NULL) return NULL; \
		retval->id = Object::create(d,retval->type()); \
		retval->refs = 0; \
		Cache::add(retval,true); \
		return retval; \
	} \
	x* x::cache_alloc(long long objid) { \
		x* retval = new x; \
		if (retval == NULL) return NULL; \
		retval->id = objid; \
		retval->refs = 0; \
		Cache::add(retval,false); \
		return retval; \
	} \

#define END }
#define INIT_METHOD(x) bool x::init(const string& s) { \
	cerr << "[" #x "::init] With string: " << s << endl; \
	map<string,string> o = parse(s); \
	cerr << "[" #x "::init] type: " << type() << " vs. " << o["type"] << endl; \
	if (o["type"] != type()) return false;

#define TOS_METHOD(x) string x::tos() {
class Cache;

class Object {
	friend class Cache;
	public:
		long long id;
		int refs;

		static long long create(Database* d, const string& s);
		bool save(Database* d);
		bool load();
		const string type();

		static map<string,string> parse(const string& s);
		template <class T> static string encode(list<T*> l);
		template <class T> static list<T*> decode(const string& s);
		
		virtual bool init(const string& s);
		virtual string tos();

	protected:
		Object();
		Object(const Object&);
		~Object();
		Object& operator=(const Object&);
};

typedef Object* ID;

// Cache Class

class Cache : public Object {
	public:
		static bool create(const string& s);
		static bool fini();	
		static bool store();
		static long long next();

		static bool add(ID obj, bool alloced);
		static bool update(Message& m);

		template<class T> static T* find(long long objid);
		template<class T> static T* find(const string& s);
		

		static string statistics();
	
		bool init(const string& s); 
		string tos(); 

	protected: \
		Cache(); \
		Cache(const Cache&);\
		~Cache(); \
		Cache& operator=(const Cache&); \
		
	private:
		static Cache* cache;
		long long next_guid;
		long long idspace;
		map<long long,ID> obj_id_map;
		map<string,int> cached;
		map<string,int> allocated;
};

template<class T> T* Cache::find(long long objid) {
	cerr << "[Cache::find] Looking up id " << objid << endl;
	ID obj = cache->obj_id_map[objid];
	if (obj != NULL) {
		obj->refs += 1;
		return static_cast<T*>(obj);
	}
	T* tobj = T::cache_alloc(objid);
	cerr << "[Cache::find] Not in cache, loading " << objid << endl;
	if(tobj->load()) {
		cerr << "[Cache::find] Loaded " << objid << endl;
		cache->cached[tobj->type()] += 1;
		tobj->refs += 1;
		return tobj;
	}
	delete tobj;
	cerr << "[Cache::find] did not load " << objid << " returning NULL" << endl;
	return NULL;
}

template<class T> T* Cache::find(const string& s) {
	if (s.empty()) {
		cerr << "[Cache::find] got empty string returning NULL" << endl;
		return (T*) NULL;
	}
	cerr << "[Cache::find] looking up " << s << endl;
	return find<T>(Uint64_of_string(s));
}

template <class T> string Object::encode(list<T*> l) {
	string retval;
	typename list<T*>::iterator i;
	for ( i = l.begin(); i != l.end(); ++i) {
		if (retval.empty()) {
			if (*i == NULL) {
				retval = ",";
			} else {
				retval = string_of_Uint64((*i)->id);
			}
		} else {
			if (*i == NULL) {
				retval += ",";
			} else {
				retval += "," + string_of_Uint64((*i)->id);
			}
		}
	}
	return retval;
};


template <class T> list<T*> Object::decode(const string& s) {
	list<T*> retval;
	size_t i,o;		
	o = 0;
	for (i = 0; i < s.size(); ++i) {
		if (s[i] != ',') continue;
		string ids = s.substr(o,i-o);
		if (ids.empty()) {
			cerr << "[Object::decode] loading NULL entry" << endl;
			retval.push_back(NULL);
		} else {
			T* ptr = Cache::find<T>(s.substr(o,i-o));
			if (ptr == NULL) {
				cerr << "[Object::decode] Failed to find id " << s.substr(o,i-o) << endl;
				retval.push_back(NULL);
			} else {
				retval.push_back(ptr);
			}
		}
		o = (i+1);	
	}
	if ( i > o ) {
		T* ptr = Cache::find<T>(s.substr(o,i-o));
		if (ptr == NULL) {
			cerr << "[Object::decode] Failed to find id " << s.substr(o,i-o) << endl;
			retval.push_back(NULL);
		} else {
			retval.push_back(ptr);
		}
	}
	return retval;
};

#endif
