// data.h
//
//	Copyright (C) 2005 David J. Goehrig 	
//	All Rights Reserved.
//
// poker game data wrapper classes
//

#ifndef __HAVE_DATA_H__
#define __HAVE_DATA_H__

#include <typeinfo>

#include <string>
#include <list>
#include <map>
#include <iostream>
using namespace std;

#include "database.h"
#include "util.h"

#define UInt64 long long

#define COMMON_METHODS(x) \
		static x* alloc(Database* d); \
		bool init(const string& s); \
		string tos(); \
	protected: \
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
		retval->id = Indexable::create(d); \
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

class Cache;
class CS;
class Event;
class League;
class Member;
class Message;
class Options;
class Player;
class Post;
class Replay;
class Season;
class Socket;
class Table;
class Team;
class Tournament;

class Indexable {
	friend class Cache;
	public:
		UInt64 id;
		int refs;

		static UInt64 create(Database* d);
		bool save(Database* d);
		bool load();
		const string type();

		static map<string,string> parse(const string& s);
		template <class T> static string encode(list<T*> l);
		template <class T> static list<T*> decode(const string& s);

		virtual bool init(const string& s);
		virtual string tos();

	protected:
		Indexable();
		Indexable(const Indexable&);
		~Indexable();
		Indexable& operator=(const Indexable&);
};

typedef Indexable* ID;

class Administrators : public Indexable {
	public:
		list<Member*>  members;

	COMMON_METHODS(Administrators)
};

class Bank : public Indexable {
	public:
		enum Pool { LEAGUE, SEASON, FREEBIE } pool;
		Player* player;
		Event* event;
		int chips;

	COMMON_METHODS(Bank)
};

class Cashier : public Indexable {
	public:
		enum Pool { LEAGUE, SEASON, FREEBIE } pool;
		enum Kind { CASH, TOURNEY, FREE } kind;
		Player* player;
		Event* event;
		int chips;
		int cost;

	COMMON_METHODS(Cashier)
};	

class CS : public Indexable {
	public:
		string name;
		string ipaddr;
		int port;
		int nat_port;
		Administrators* admins;

	COMMON_METHODS(CS);
};

class Details : public Indexable {
	public:
		string name;
		string email;
		string phone;
		string cell;
		string street;
		string apartment;
		string city;
		string state;
		string zip;
		string country;
		string dob;
		string sex;
		string occupation;
		string income;
		string spam;
	
	COMMON_METHODS(Details)
};

class Event : public Indexable {
	public:
		string name;
		string notes;
		string game;
		int date;
		bool playoff;
		Player* owner;
		Season* season;
		Options* options;
		list<Cashier*> cashiers;
		list<Bank*> banks;
		list<Table*> tables;
		Tournament* tournament;

		int create_tables();
		int prune_tables();
		static int max_seats[];

	COMMON_METHODS(Event)
};

class Friend : public Indexable {
	public:
		Player* player;
		string category;
		string rating;
		string personality;
		string notes;	
	
	COMMON_METHODS(Friend)
};

class Seat {
	public:
		Player* player;
		int seat;
		int purse;

		Seat();
		~Seat();

		string encode();
		bool decode(const string& s);
};

class Results {
	public:
		string winnings;
		list<string> actions;

		Results();
		~Results();
		
		string encode();
		bool decode(const string& s);
};

class Game : public Indexable {
	public:
		static const int MAX_SEATS = 9;

		bool done;
		string game;
		Event* event;
		Table* table;
		Replay* replay;
		Seat seats[MAX_SEATS];
		Results results[MAX_SEATS];
		list<string> pots;
	
	COMMON_METHODS(Game)
};

class Invite : public Indexable {
	public:
		enum Kind { FRIEND, LEAGUE, TEAM, EVENT, ACCEPTED } kind;
		Player* sender;	
		union {
			League* league;
			Team* team;
			Event* event;
		} target;
		enum Destination { PLAYER, EMAIL } dest;
		struct {
			Player* player;
			struct {
				string name;
				string address;
			} email; 
		} recipient;
		string message;

	COMMON_METHODS(Invite)
};

class League : public Indexable {
	public:
		string name;
		int num_members;
		Player* commish;
		list<Season*> seasons;
		list<Member*> members;
		list<Invite*> invites;
		list<Post*> posts;

	COMMON_METHODS(League)
};

#define LOGIN_CHECK if(!c->isBot() && (c->login() == NULL || c->login()->player == NULL)) return false;
#define BOT_CHECK if (! c->isBot()) return false;

class Login : public Indexable {
	public:
		string uid;
		Player* player;
		int seat;
		CS* server;	
		Socket* sock;	// DYNAMIC NOT SAVED
		Replay* replay;
		int frame;

	COMMON_METHODS(Login);
};

class Member : public Indexable {
	public:
		Player* player;
		string access;
	
	COMMON_METHODS(Member)
};

class Options : public Indexable {
	public:
//		enum WinType { CHIPS, POSITION } win;
		int value;
		enum LimitType { LIMIT, POTLIMIT, NOLIMIT } limit;
		enum BetStages { ONE, TWO } stages;
		struct {
			int min;
			int max;
			int no;
		} buyin, bet[2];	
		int ante;
		struct {	
			int small;
			int big;
		} blinds;
		string wildcards;
		int purchase;
		int hilo;

	COMMON_METHODS(Options)
};

class Player : public Indexable {
	public:
		string username;
		string password;
		string avatar;
		Details* details;
		list<League*> leagues;
		list<Friend*> friends;
		list<Invite*> invites;
		list<Cashier*> cashiers;
		list<Bank*> banks;
		map<UInt64,Member*> league_member_map;

	COMMON_METHODS(Player)
};

class Post : public Indexable {
	public:
		Player* poster;
		Post* parent;
		list<Post*> replies;
		string subject;
		int date;
		int last;
		string body;

		string display();

	COMMON_METHODS(Post)	
};

class Replay : public Indexable {
	private:
		static Player* default_player;
	public:
		list<string> messages;

		bool add(Message& m);
		bool send(Socket* s, int frame);

		static Player* player();

	COMMON_METHODS(Replay);

};

class HSP {
	public:
		Player* p;
		string h;

		HSP();
		HSP(Player* a, const string& s);
		~HSP();
		static string encode(list<HSP> l);
		static list<HSP> decode(const string& s);
};

class PVP {
	public:
		Player* p;
		Player* op;

		PVP();
		PVP(Player* a, Player* b);
		~PVP();
		static string encode(list<PVP> l);
		static list<PVP> decode(const string& s);
};

class Season : public Indexable {
	public:
		string name;
		enum SeasonState { PRESEASON, ACTIVE, POSTSEASON } state;
		League* league;
		Event* current;
		list<Event*> events;
		list<Event*> playoffs;
		string has_playoffs;
		string ranking;
		string qualifiers;
		list<Team*> teams;
		list<Player*> stats;
		list<HSP> holdem_stats;
		list<PVP> opp_stats;
		list<Post*> posts;

		bool addEvents(list<Event*>& el);
		bool addPlayoffs(list<Event*>& el, string qual);
		bool hasStats(Player* p);
		bool hasHoldemStats(Player* p, const string& h);
		bool hasOppStats(Player*p, Player* o);
	
	COMMON_METHODS(Season)
};

class Table : public Indexable {
	public:
		static const int MAX_SEATS = 9;

		string name;
		Event* event;
		list<Game*> games;
		Game* current;
		Seat seats[MAX_SEATS];

	COMMON_METHODS(Table)
};

class Team : public Indexable {
	public:
		string name;
		Player* captain;
		list<Player*> players;
		int points;
		int position;

	COMMON_METHODS(Team)
};

struct round {
	int duration;
	int small_blind;
	int big_blind;
	int ante;
	int break_time;
};
class tournament_seating{
	public:
		Player* player;
		Table* table;
		int seat;
		int purse;
		int position;

		tournament_seating();
		~tournament_seating();
		bool operator==(const tournament_seating& ts);
}; 

class Tournament : public Indexable {
	public:
		int current;
		int cost;
		int chips;
		struct {
			int cost;
			int chips;
			int threshold;
			int end_round;
		} rebuys;
		struct {
			int cost;
			int chips;
		} addon;
		list<tournament_seating> seating;
		list<struct round> rounds;

		bool add_player(Player* p);
		bool remove_player(Player* p);
		bool update_player(Player* p, Table* t, int seat, int chips, int position);
		bool seat_players(list<Table*>& t);
		string get_seating();
		bool fabricate_rounds();
		bool is_registered(Player* p);

	COMMON_METHODS(Tournament)
};

class Cache : public Indexable {
	public:
		static bool create(const string& s);
		static bool fini();	
		static bool store();
		static UInt64 next();

		static bool add(ID obj, bool alloced);
		static bool update(Message& m);

		template <class T> static T* find(UInt64 i);
		template <class T> static T* find(const string& s);

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
		UInt64 next_guid;
		UInt64 idspace;
		map<UInt64,ID> obj_id_map;
		map<string,int> cached;
		map<string,int> allocated;
};

// Instantiate template functions

template <class T>
string
Indexable::encode(list<T*> l)
{
	string retval;
	typename list<T*>::iterator i;
	for ( i = l.begin(); i != l.end(); ++i) {
		if (retval.empty()) {
			if (*i == NULL) {
				retval = "|";
			} else {
				retval = string_of_Uint64((*i)->id);
			}
		} else {
			if (*i == NULL) {
				retval += "|";
			} else {
				retval += "|" + string_of_Uint64((*i)->id);
			}
		}
	}
	return retval;
}

template <class T> 
list<T*>
Indexable::decode(const string& s) 
{
	list<T*> retval;
	size_t i,o;		
	o = 0;
	for (i = 0; i < s.size(); ++i) {
		if (s[i] != '|') continue;
		string ids = s.substr(o,i-o);
		if (ids.empty()) {
			cerr << "[Indexable::decode] loading NULL entry" << endl;
			retval.push_back(NULL);
		} else {
			UInt64 dbid = Uint64_of_string(s.substr(o,i-o));
			cerr << "[Indexable::decode] loading " << dbid << endl;
			T* ptr = Cache::find<T>(dbid);
			if (ptr == NULL) {
				cerr << "[Indexable::decode] Failed to find id " << dbid << endl;
				retval.push_back(NULL);
			} else {
				retval.push_back(ptr);
			}
		}
		o = (i+1);	
	}
	if ( i > o ) {
		UInt64 dbid = Uint64_of_string(s.substr(o,i-o));
		T* ptr = Cache::find<T>(dbid);
		if (ptr == NULL) {
			cerr << "[Indexable::decode] Failed to find id " << dbid << endl;
			retval.push_back(NULL);
		} else {
			retval.push_back(ptr);
		}
	}
	return retval;
}

template<class T> T* Cache::find(UInt64 objid)
{
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

template<class T> T* Cache::find(const string& s)
{
	if (s.empty()) {
		cerr << "[Cache::find] got empty string returning NULL" << endl;
		return (T*) NULL;
	}
	cerr << "[Cache::find] looking up " << s << endl;
	return find<T>(Uint64_of_string(s));
}



#endif
