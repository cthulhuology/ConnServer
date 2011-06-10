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
using namespace std;

#include "database.h"
#include "util.h"
#include "message.h"
#include "user_map.h"
#include "socket.h"

#define UInt64 long long

#define PROPERTY(x) x = o[#x];
#define INT_PROPERTY(x) x = int_of_string(o[#x]);
#define OBJ_PROPERTY(x,y) x = Cache::find<y>(o[#x]);
#define LIST_PROPERTY(x,y) x = decode<y>(o[#x]);

#define COMMON_METHODS(x) \
		static x* alloc(Database* d); \
		bool init(const string& s); \
		string tos(); \
		static x* cache_alloc(UInt64 objid); \
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
	x* x::cache_alloc(UInt64 objid) { \
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

class Object {
	friend class Cache;
	public:
		UInt64 id;
		int refs;

		static UInt64 create(Database* d, const string& s);
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

#endif
