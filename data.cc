// data.cc
//
//	Copyright (C) 2005,2010 David J. Goehrig
//	All Rights Reserved.
//
// data wrapper classes
//

#include "data.h"
#include "database.h"
#include "util.h"
#include "message.h"
#include "user_map.h"
#include "esocket.h"

// Object Class

Object::Object() { }
Object::~Object() { }

bool Object::init(const string& s) { 
	cerr << "[Object::init] " << s << endl;
	return true; 
}
string Object::tos() { return string(""); }

UInt64 Object::create(Database* d, const string& t) {
	UInt64 retval = 0LL;
	string query = "SELECT * FROM new_object('" + t + "','')";
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

string
escape_tick(const string& s)
{
	string retval;
	int i;
	for (i = 0; i < s.size(); ++i) {
		if (retval.empty()) {
			if ( s[i] == '\'' ) {
				retval = "%27";
			} else {
				retval = s[i];
			}
		} else {
			if ( s[i] == '\'' ) {
				retval += "%27";
			} else {
				retval += s[i];
			}
		}
	}
	return retval;
}

bool 
Object::save(Database* d)
{
	bool retval = true;
	string query = "SELECT * FROM save_object(" + string_of_Uint64(id) + ",'" + escape_tick(tos()) + "')";
	cerr << "[Object::save] " << query << endl;
	Result* res = d->query(query);	
	if (res == NULL || ! res->success()) {
		cerr << "[Object::save] Failed to save object " << id << " because: " << d->error() << endl;
		retval = false;
	}
	delete res;
	return retval;
}

bool 
Object::load()
{
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

const string
Object::type()
{
	return string(typeid(*this).name());
}

map<string,string>
Object::parse(const string& s)
{
	map<string,string> retval;
	list<string> kv_pairs = split(',',s);
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

// Administrators Class

ALLOC_METHODS(Administrators)

INIT_METHOD(Administrators)
	players = decode<Player>(o["players"]);
END

TOS_METHOD(Administrators)
	return "type:" + type() + ",players=" + encode<Player>(players);	
END

// Details Class

ALLOC_METHODS(Details)

INIT_METHOD(Details)
	name = o["name"];
	email = o["email"];
	phone = o["phone"];
	cell = o["cell"];
	street = o["street"];
	apartment = o["apartment"];
	city = o["city"];
	state = o["state"];
	zip = o["zip"];
	country = o["country"];
	dob = o["dob"];
	sex = o["sex"];
	occupation = o["occupation"];
	income = o["income"];
	spam = o["spam"];
	return true;
END

TOS_METHOD(Details)
	return "type:" + type() 
		+ "&name:" + name 
		+ "&email:" + email 
		+ "&phone:" + phone 
		+ "&cell:" + cell 
		+ ",street:" + street 
		+ ",apartment:" + apartment 
		+ ",city:" + city 
		+ ",state:" + state 
		+ ",zip:" + zip 
		+ ",country:" + country 
		+ ",dob:" + dob 
		+ ",sex:" + sex 
		+ ",occupation:" + occupation 
		+ ",income:" + income 
		+ ",spam:" + spam;
END

// Event Class

ALLOC_METHODS(Event)

INIT_METHOD(Event)
	name = o["name"];
	params = o["params"];
	date = int_of_string(o["date"]);
	return true;
END

TOS_METHOD(Event)
	return "type:" + type() 
		+ ",name:" + name 
		+ ",params:" + params 
		+ ",date:" + string_of_int(date); 
END

// Friend Class

ALLOC_METHODS(Friend)

INIT_METHOD(Friend)
	player = Cache::find<Player>(o["player"]);
	category = o["category"];
	rating = o["rating"];
	personality = o["personality"];
	notes = o["notes"];
	return player != NULL;
END

TOS_METHOD(Friend)
	return "type:" + type() 
		+ ",player:" + (player == NULL ? "" : string_of_Uint64(player->id)) 
		+ ",category:" + category
		+ ",rating:" + rating 
		+ ",personality:" + personality 
		+ ",notes:" + notes;
END

// Login Class

Login::Login() {}
Login::~Login() {}

Login*
Login::alloc(Database* d)
{
	Login* retval = new Login();
	return retval;
}

INIT_METHOD(Login)
	player = Cache::find<Player>(o["player"]);
	server = Cache::find<CS>(o["server"]);
	sock = NULL;
	return player != NULL && sock != NULL;
END

TOS_METHOD(Login)
	return "type:" + type() 
		+ ",player:" + (player == NULL ? "" : string_of_Uint64(player->id))
		+ ",server:" + (server == NULL ? "" : string_of_Uint64(server->id));
END

// Player Class

ALLOC_METHODS(Player)

INIT_METHOD(Player)
	username = o["username"];
	password = o["password"];
	avatar = o["avatar"];
	details = Cache::find<Details>(o["details"]);
	friends = decode<Friend>(o["friends"]);
	posts = decode<Post>(o["posts"]);
	return details != NULL;
END

TOS_METHOD(Player)
	return "type:" + type() 
		+ ",username:" + username 
		+ ",avatar:" + avatar 
		+ ",password:" + password 
		+ ",details:" + ( details == NULL ? "" : string_of_Uint64(details->id))
		+ ",friends:" + encode<Friend>(friends)
		+ ",posts:" + encode<Post>(posts);
END


// Post Class

ALLOC_METHODS(Post)

INIT_METHOD(Post)
	poster = Cache::find<Player>(o["poster"]);
	parent = Cache::find<Post>(o["parent"]);
	replies = decode<Post>(o["posts"]);
	subject = o["subject"];
	date = int_of_string(o["date"]);
	last = int_of_string(o["last"]);
	body = o["body"];
END

TOS_METHOD(Post)
	return "type:" + type() 
		+ ",poster:" + (poster == NULL ? "" : string_of_Uint64(poster->id))
		+ ",parent:" + (parent == NULL ? "" : string_of_Uint64(parent->id))
		+ ",replies:" + encode<Post>(replies)
		+ ",subject:" + subject
		+ ",date:" + string_of_int(date)
		+ ",last:" + string_of_int(last)
		+ ",body:" + body;
END

string Post::display() {
	return "{ " + body + " }";
}

// CS Class

ALLOC_METHODS(CS)

INIT_METHOD(CS)
	name = o["name"];
	ipaddr = o["ipaddr"];
	port = int_of_string(o["port"]);
	nat_port = int_of_string(o["nat_port"]);
	admins = Cache::find<Administrators>(o["admins"]);
	return true;
END

TOS_METHOD(CS)
	return "type:" + type() 
		+ ",name:" + name 
		+ ",ipaddr:" + ipaddr 
		+ ",port:" + string_of_int(port) 
		+ ",nat_port:" + string_of_int(nat_port);
END

// Cache Class

Cache* Cache::cache = NULL;

Cache::Cache() {};
Cache::~Cache() {};

bool 
Cache::create(const string& s)
{
	if (cache != NULL) return false;

	cache = new Cache();
	
	string load_cache = "SELECT value FROM objects WHERE id = " + s;
	Database* d = DBPool::grab();
	d->begin();
	Result* res = d->query(load_cache);

	if (res == NULL || ! res->success() || res->rows != 1) {
		cerr << "[Cache::init] failed " << d->error() << endl;
		cache->next_guid = Uint64_of_string(s);
		cache->idspace = Uint64_of_string(s);
		cache->id = Object::create(d,cache->type());
		cache->save(d);
		d->commit();
		delete res;
		DBPool::release(d);
		return true;
	} else {
		string c = (*res)["value"][0];
		d->commit();
		delete res;
		DBPool::release(d);
		cache->id = Uint64_of_string(s);
		return cache->init(c);
	}
}

bool
Cache::init(const string& s)
{
	map<string,string> o = parse(s);
	idspace = Uint64_of_string(o["idspace"]);
	next_guid = Uint64_of_string(o["next_guid"]);
	map<string,string>::iterator i;
	for (i = o.begin(); i != o.end(); ++i) {
		if (i->first != "idspace" && i->first != "next_guid" && i->first != "type") {
			allocated[i->first] = int_of_string(i->second);
		}
	}
	return true;
END

TOS_METHOD(Cache)
	string stats;
	map<string,int>::iterator i;
	for (i = allocated.begin(); i != allocated.end(); ++i) {
		if (stats.empty()) {
			stats = "," + i->first + ":" + string_of_int(i->second);
		} else {
			stats += "," + i->first + ":" + string_of_int(i->second);
		}
	}
	return "type:" + type()  
		+ ",idspace:" + string_of_Uint64(idspace)
		+ ",next_guid:" + string_of_Uint64(next_guid)
		+ stats;
END

bool
Cache::fini()
{
	if (cache == NULL) return false;
	cerr << "[Cache::fini] " << cache->tos() << endl;
	Database* d = DBPool::grab();
	d->begin();
	if (!cache->save(d) || ! d->commit()) {
		d->rollback();
		return false;
	}
	map<UInt64,ID>::iterator i;
	for (i = cache->obj_id_map.begin(); i != cache->obj_id_map.end(); ++i) {
		delete i->second;
	}
	DBPool::release(d);
	delete cache;
	cache = NULL;
	return true;
}

bool
Cache::store()
{
	if (cache == NULL) return false;
	bool retval = true;
	map<UInt64,ID>::iterator i;
	Database* d = DBPool::grab();
	d->begin();
	for (i = cache->obj_id_map.begin(); i != cache->obj_id_map.end(); ++i) {
		if (i->second) {
			retval &= i->second->save(d);
		}
	}
	retval &= cache->save(d);
	if (retval) {
		d->commit();
	} else {
		d->rollback();
	}
	DBPool::release(d);
	return retval;
}

UInt64
Cache::next()
{
	Database* d = DBPool::grab();
	d->begin();
	UInt64 retval = cache->next_guid;
	cache->next_guid += 1;
	if (! cache->save(d) || ! d->commit()) {
		cerr << "[Cache::next] failed to allocate guid" << endl;
		d->rollback();
		DBPool::release(d);
		return Cache::next();
	}
	DBPool::release(d);
	cerr << "[Cache::next] allocating " << retval << endl;
	return retval;
}

bool
Cache::add(ID obj, bool alloced)
{
	if (obj == NULL || obj->id == 0LL) return false;
	cache->obj_id_map[obj->id] = obj;
	if (alloced) cache->allocated[obj->type()] += 1;
	return true;
}

bool
Cache::update(Message&  m)
{
	if (m["id"].empty() || m["obj"].empty()) return false;
	UInt64 objid = Uint64_of_string(m["id"]);
	ID obj = cache->obj_id_map[objid];
	if (obj == NULL) return true; // Object not in cache
	obj->init(m["obj"]);
}

string
Cache::statistics()
{
	string stats = "msg=cache-stats";
	map<string,int>::iterator i,j;
	for (i = cache->allocated.begin(); i != cache->allocated.end(); ++i) {
		stats += "&" + i->first + ".allocated=" 
			+ string_of_int(i->second);
	}
	for (j = cache->cached.begin(); j != cache->cached.end(); ++j) {
		stats += "&" + j->first + ".cached=" 
			+ string_of_int(j->second);
	}
	return stats;
}
