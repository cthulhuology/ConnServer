// data.cc
//
//	Copyright (C) 2005,2010 David J. Goehrig
//	All Rights Reserved.
//
// game data wrapper classes
//

#include "data.h"
#include "database.h"
#include "util.h"
#include "message.h"
#include "user_map.h"
#include "esocket.h"

// Indexable Class

Indexable::Indexable() { }
Indexable::~Indexable() { }

bool Indexable::init(const string& s) { 
	cerr << "[Indexable::init] " << s << endl;
	return true; 
}
string Indexable::tos() { return string(""); }

UInt64 
Indexable::create(Database* d)
{
	UInt64 retval = Cache::next();
	string query = "INSERT INTO cache_table (id) VALUES (" + string_of_Uint64(retval) + ")";
	cerr << "[Indexable::create] " << query << endl;
	Result* res = d->query(query);
	if (res == NULL || ! res->success()) {
		cerr << "[Indexable::create] Failed to allocate storage: " << d->error() << endl;
		retval = 0LL;
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
Indexable::save(Database* d)
{
	bool retval = true;
	string query = "UPDATE cache_table SET value = '" + escape_tick(tos()) + "' WHERE id = " + string_of_Uint64(id);
	cerr << "[Indexable::save] " << query << endl;
	Result* res = d->query(query);	
	if (res == NULL || ! res->success()) {
		cerr << "[Indexable::save] Failed to save object " << id << " because: " << d->error() << endl;
		retval = false;
	}
	delete res;
	return retval;
}

bool 
Indexable::load()
{
	bool retval = true;
	string query = "SELECT id, value FROM cache_table WHERE id = " + string_of_Uint64(id);
	cerr << "[Indexable::load] " << query << endl;
	Database* d = DBPool::grab();
	cerr << "[Indexable::load] Database is " << d << endl;
	d->begin();
	Result* res = d->query(query);
	cerr << "[Indexable::load] Result is " << res << endl;
	d->commit();
	DBPool::release(d);
	cerr << "[Indexable::load] Result is " << (res->success() ? "Success!" : "Failure!") << endl;
	if (res == NULL || ! res->success()) {
		cerr << "[Indexable::load] Failed to load object " << id << " because: " << d->error() << endl;
		retval = false;
	} else if ( res->rows == 0 ) {
		cerr << "[Indexable::load] Object " << id << " not found" << endl;
		retval = false;
	} else if ( res->rows > 1) {
		cerr << "[Indexable::load] Waring multiple objects " << id << " database corrupted" << endl;
		retval = false;
	} else {
		cerr << "[Indexable::load] Init with value: " << (*res)["value"][0]  << endl;
		retval = init((*res)["value"][0]);
	}
	delete res;
	cerr << "[Indexable::load] released" << endl;
	return retval;
}

const string
Indexable::type()
{
	return string(typeid(*this).name());
}

map<string,string>
Indexable::parse(const string& s)
{
	map<string,string> retval;
	list<string> kv_pairs = split('&',s);
	list<string>::iterator i;
	for (i = kv_pairs.begin(); i != kv_pairs.end(); ++i) {
	//	cerr << "--- " << *i << endl;
		list<string> kv = split('=',*i);
		if (kv.size() >= 2 ) {
			string first = kv.front();
			kv.pop_front();
			string second = kv.front();
			retval[first] = second;
	//		cerr << "KEY=VALUE: " << first << "=" << second << endl;
		} else if (kv.size() == 1) {
			retval[kv.front()] = "";
		}
	}
	return retval;
}

// Administrators Class

ALLOC_METHODS(Administrators)

INIT_METHOD(Administrators)
	members = decode<Member>(o["members"]);
END

TOS_METHOD(Administrators)
	return "type=" + type() + "&members=" + encode<Member>(members);	
END

// Bank Class

ALLOC_METHODS(Bank)

INIT_METHOD(Bank)
	pool =  static_cast<Pool>(int_of_string(o["pool"]));
	player = Cache::find<Player>(o["player"]);
	event = Cache::find<Event>(o["event"]);
	chips = int_of_string(o["chips"]);
	return event != NULL && player != NULL;
END

TOS_METHOD(Bank)
	return "type=" + type() 
		+ "&pool=" + string_of_int(pool)
		+ "&player=" + (player == NULL ? "" :  string_of_Uint64(player->id))
		+ "&event=" + (event == NULL ? "" : string_of_Uint64(event->id))
		+ "&chips=" + string_of_int(chips);
END

//  Cashier Class

ALLOC_METHODS(Cashier)

INIT_METHOD(Cashier)
	pool = static_cast<Pool>(int_of_string(o["pool"]));
	kind = static_cast<Kind>(int_of_string(o["kind"]));
	player = Cache::find<Player>(o["player"]);
	event = Cache::find<Event>(o["event"]);
	chips = int_of_string(o["chips"]);
	cost = int_of_string(o["cost"]);
	return true;
END

TOS_METHOD(Cashier)
	return "type=" + type() 
		+ "&kind=" + string_of_int(kind) 
		+ "&pool=" + string_of_int(pool) 
		+ "&player=" + (player ==  NULL ? "" : string_of_Uint64(player->id))
		+ "&event=" + (event == NULL ? "" : string_of_Uint64(event->id))
		+ "&chips=" + string_of_int(chips)
		+ "&cost=" + string_of_int(cost);
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
	return "type=" + type() + "&name=" + name + "&email=" + email + "&phone=" + phone + "&cell=" + cell +
		"&street=" + street + "&apartment=" + apartment + "&city=" + city + "&state=" + state + "&zip=" + zip +
		"&country=" + country + "&dob=" + dob + "&sex=" + sex + "&occupation=" + occupation + "&income=" +
		income + "&spam=" + spam;
END

// Event Class

ALLOC_METHODS(Event)

INIT_METHOD(Event)
	name = o["name"];
	notes = o["notes"];
	game = o["game"];
	date = int_of_string(o["date"]);
	playoff = int_of_string(o["playoff"]);
	owner = Cache::find<Player>(o["owner"]);
	season = Cache::find<Season>(o["season"]);
	options = Cache::find<Options>(o["options"]);
	tables = decode<Table>(o["tables"]);
	cashiers = decode<Cashier>(o["cashiers"]);
	banks = decode<Bank>(o["banks"]);
	tournament = Cache::find<Tournament>(o["tournament"]);
	return true;
END

TOS_METHOD(Event)
	return "type=" + type() + "&name=" + name 
		+ "&notes=" + notes 
		+ "&game=" + game + "&date=" + string_of_int(date) 
		+ "&playoff=" + string_of_int(playoff)
		+ "&owner=" + (owner == NULL ? "" : string_of_Uint64(owner->id))
		+ "&season=" + (season == NULL ? "" : string_of_Uint64(season->id))
		+ "&options=" + (options == NULL ? "" : string_of_Uint64(options->id))
		+ "&tables=" + encode<Table>(tables)
		+ "&cashiers=" + encode<Cashier>(cashiers)
		+ "&banks=" + encode<Bank>(banks)
		+ "&tournament=" + (tournament == NULL ? "" : string_of_Uint64(tournament->id));;
END

int Event::max_seats[] = {
			9, // dealer's choice 0
			9, // holdem 1
			9, // pineapple 2
			7, // King Little 3
			9, // 5 card stud 4
			9, // 5 card draw 5
			7, // 7 card stud 6
			9, // anaconda 7
			7, // follow the queen 8
			7, // black mariah 9
			9, // omaha 10
			9, // goodbadugly 11
			7, // underwater baseball 12
		};

int Event::create_tables()
{
	int retval = 0;
	Database* d = DBPool::grab();
	d->begin();
	if (tournament == NULL) {
		if (season == NULL) {
			retval = 1;
		} else {
			retval = 1 + season->league->num_members / max_seats[int_of_string(game)];
		}	
	} else {
		list<tournament_seating>::iterator j;
		int num_players = 0;
		for (j = tournament->seating.begin(); j != tournament->seating.end(); ++j) {
			if ( j->player == NULL) continue;
			if ( j->purse > 0)  ++num_players;
		}
		retval = 1 + num_players / max_seats[int_of_string(game)];
	}
	for (int i = 0; i < retval; ++i) {
		Table* tab = Table::alloc(d);
		for (int j = 0; j < Table::MAX_SEATS; ++j) {
			tab->seats[j].player = NULL;
			tab->seats[j].seat = -1;
			tab->seats[j].purse = 0;
		}
		tab->name = name + "!" + string_of_int(i+1);
		tab->event = this;
		tab->current = NULL;
		if (! tab->save(d)) {
			cerr << "[Event::create_tables] failed to create table" << endl;
			d->rollback();
			return i;
		} else {
			tables.push_back(tab);	
		}
	}
	return retval;
}

int Event::prune_tables()
{
	return 0;
}

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
	return "type=" + type() 
		+ "&player=" + (player == NULL ? "" : string_of_Uint64(player->id)) 
		+ "&category=" + category
		+ "&rating=" + rating 
		+ "&personality=" + personality 
		+ "&notes=" + notes;
END

// Game Class

Seat::Seat() 
{
	player = NULL;
	seat = -1;
	purse = 0;	
}

Seat::~Seat()
{
}

string
Seat::encode()
{
	return (player == NULL ? "!!" : string_of_Uint64(player->id) + "!" 
		+ string_of_int(seat) + "!" + string_of_int(purse));
}

bool
Seat::decode(const string& s)
{
	player = NULL;
	seat = -1;
	purse = 0;
	if (s.empty()) return true;
	
	list<string> sl = split('!',s);
	list<string>::iterator i = sl.begin();

	if (i != sl.end() && ! (*i).empty()) {
		player = Cache::find<Player>(*i);
		++i;
	} else {
		return false;
	}
	
	if (i != sl.end() && ! (*i).empty()) {
		seat = int_of_string(*i);
		++i;
	} else {
		return false;
	}
	
	if (i != sl.end() && ! (*i).empty()) {
		purse= int_of_string(*i);
		++i;
	} else {
		return false;
	}

	return true;
}


Results::Results() 
{
	winnings = "";
	actions.clear();
}

Results::~Results()
{
}

string
Results::encode()
{
	return winnings + "," + join('!',actions);
}

bool
Results::decode(const string& s)
{
	winnings = "";
	actions.clear();

	if (s.empty())  return true;
	list<string> rsts = split(',',s);
	if (rsts.size() != 2) return true;
	winnings = rsts.front();
	rsts.pop_front();
	string acts = rsts.front();
	actions = split('!',acts);
}

ALLOC_METHODS(Game)

INIT_METHOD(Game)
	done = int_of_string(o["done"]);
	game = o["game"];
	event = Cache::find<Event>(o["event"]);
	table = Cache::find<Table>(o["table"]);
	replay = Cache::find<Replay>(o["replay"]);
	
	list<string>::iterator i;
	list<string> sts = split('|',o["seats"]);
	int v = 0;
	for (i = sts.begin(); i != sts.end(); ++i) {
		if ((*i).empty()){
			if (v != MAX_SEATS) ++v;
			continue;
		}
		if (v != MAX_SEATS) {
			seats[v].decode(*i);
			++v;
		}
	}
	
	list<string> rsts = split('|',o["results"]);
	int r = 0;
	for (i = rsts.begin(); i != rsts.end(); ++i) {
		if ((*i).empty()) {
			if ( r != MAX_SEATS) ++r;
			continue;
		}
		if (r != MAX_SEATS) {
			results[r].decode(*i);
			++r;
		}
	}
	return event != NULL;
END

TOS_METHOD(Game)

	string sts, rsts;
	
	int s;
	for( s = 0; s != MAX_SEATS; ++s) {
		if (sts.empty()) {
			sts = seats[s].encode();
		} else {
			sts += "|" + seats[s].encode();
		}
	}
	
	int r;
	for( r = 0; r != MAX_SEATS; ++r) {
		if (rsts.empty()) {
			rsts = results[r].encode();
		} else {
			rsts += "|" + results[r].encode();
		}
	}

	return "type=" + type() 
		+ "&done=" + string_of_int(done)
		+ "&game=" + game
		+ "&event=" + (event == NULL ? "" : string_of_Uint64(event->id)) 
		+ "&table=" + (table == NULL ? "" : string_of_Uint64(table->id))
		+ "&replay=" + (replay == NULL ? "" : string_of_Uint64(replay->id))
		+ "&seats=" + sts
		+ "&results=" + rsts;
END

// Invite Class

ALLOC_METHODS(Invite)

INIT_METHOD(Invite)
	sender = Cache::find<Player>(o["sender"]);
	kind = static_cast<Kind>(int_of_string(o["kind"]));
	switch (kind) {
		case LEAGUE:
			target.league = Cache::find<League>(o["league"]);
			break;
		case TEAM:
			target.team = Cache::find<Team>(o["team"]);
			break;
		case EVENT:
			target.event = Cache::find<Event>(o["event"]);
			break;
	}
	dest = static_cast<Destination>(int_of_string(o["dest"]));
	switch (dest) {
		case PLAYER:
			recipient.player = Cache::find<Player>(o["recipient"]);
			break;
		case EMAIL:
			recipient.player = NULL;
			recipient.email.address = o["recipient.email"];
			recipient.email.name = o["recipient.name"];
			break;
	}
	message = o["message"];
	return sender != NULL;
END

TOS_METHOD(Invite)
	string inv;
	switch (kind) {
		case LEAGUE:
			inv = "&league=" + (target.league == NULL ? "" :  string_of_Uint64(target.league->id));
			break;
		case TEAM:
			inv = "&team=" + (target.team == NULL ? "" : string_of_Uint64(target.team->id));
			break;
		case EVENT:
			inv = "&event=" + (target.event == NULL ? "" : string_of_Uint64(target.event->id));
			break;
	}
	string dst;
	switch (dest) {
		case PLAYER:
			dst = "&recipient=" + (recipient.player == NULL ? "" : string_of_Uint64(recipient.player->id));
			break;
		case EMAIL:
			dst = "&recipient.email=" + recipient.email.address + "&recipient.name=" + recipient.email.name;
			break;
	}
	return "type=" + type() 
		+ "&sender=" + (sender == NULL ? "" : string_of_Uint64(sender->id))
		+ "&kind=" + string_of_int(kind) + "&message=" + message + inv 
		+ "&dest=" + string_of_int(dest) + dst;
END

// League Class

ALLOC_METHODS(League)

INIT_METHOD(League)
	name = o["name"];
	num_members = int_of_string(o["num_members"]);
	commish = Cache::find<Player>(o["commish"]);	
	seasons = decode<Season>(o["seasons"]);
	members = decode<Member>(o["members"]);
	invites = decode<Invite>(o["invites"]);
	posts = decode<Post>(o["posts"]);
	return commish != NULL;
END

TOS_METHOD(League)
	return "type=" + type() + "&name=" + name 
		+ "&num_members=" + string_of_int(num_members)
		+ "&commish=" + (commish == NULL ? "" : string_of_Uint64(commish->id)) 
		+ "&seasons=" + encode<Season>(seasons)
		+ "&members=" + encode<Member>(members) 
		+ "&posts=" + encode<Post>(posts)
		+ "&invites=" + encode<Invite>(invites);
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
	uid = o["uid"];
	player = Cache::find<Player>(o["player"]);
	server = Cache::find<CS>(o["server"]);
	sock = NULL;
	return player != NULL && sock != NULL;
END

TOS_METHOD(Login)
	return "type=" + type() 
		+ "&uid=" + uid 
		+ "&player=" + (player == NULL ? "" : string_of_Uint64(player->id))
		+ "&server=" + (server == NULL ? "" : string_of_Uint64(server->id));
END

// Member Class

ALLOC_METHODS(Member)

INIT_METHOD(Member)
	player = Cache::find<Player>(o["player"]);
	access = o["access"];
	return player != NULL;
END

TOS_METHOD(Member)
	return "type=" + type() 
		+ "&player=" + (player == NULL ? "" : string_of_Uint64(player->id))
		+ "&access=" + access;
END

// Options Class

ALLOC_METHODS(Options)

INIT_METHOD(Options)
//	win = static_cast<WinType>(int_of_string(o["win"]));
	limit = static_cast<LimitType>(int_of_string(o["limit"]));
	stages = static_cast<BetStages>(int_of_string(o["stages"]));
	value = int_of_string(o["value"]);
	buyin.no = int_of_string(o["buyin.no"]);
	buyin.min = int_of_string(o["buyin.min"]);
	buyin.max = int_of_string(o["buyin.max"]);
	bet[0].no = int_of_string(o["bet[0].no"]);
	bet[0].min = int_of_string(o["bet[0].min"]);
	bet[0].max = int_of_string(o["bet[0].max"]);
	bet[1].no = int_of_string(o["bet[1].no"]);
	bet[1].min = int_of_string(o["bet[1].min"]);
	bet[1].max = int_of_string(o["bet[1].max"]);
	ante = int_of_string(o["ante"]);
	blinds.small = int_of_string(o["blinds.small"]);	
	blinds.big = int_of_string(o["blinds.big"]);	
	wildcards = o["wildcards"];
	hilo = int_of_string(o["hilo"]);
	purchase = int_of_string(o["purchase"]);
END

TOS_METHOD(Options)
	return "type=" + type() + "&limit=" + string_of_int(limit) +
		"&stages=" + string_of_int(stages) 
		+ "&buyin.no=" + string_of_int(buyin.no)
		+ "&buyin.min=" + string_of_int(buyin.min)
		+ "&buyin.max=" + string_of_int(buyin.max)
		+ "&bet[0].no=" + string_of_int(bet[0].no)
		+ "&bet[0].min=" + string_of_int(bet[0].min)
		+ "&bet[0].max=" + string_of_int(bet[0].max)
		+ "&bet[1].no=" + string_of_int(bet[1].no)
		+ "&bet[1].min=" + string_of_int(bet[1].min)
		+ "&bet[1].max=" + string_of_int(bet[1].max)
		+ "&ante=" + string_of_int(ante)
		+ "&blinds.small=" + string_of_int(blinds.small)
		+ "&blinds.big=" + string_of_int(blinds.big)
		+ "&wildcards=" + wildcards
		+ "&purchase=" + string_of_int(purchase)
//		+ "&win=" + string_of_int(win)
		+ "&hilo=" + string_of_int(hilo)
		+ "&value=" + string_of_int(value);
END

// Player Class

ALLOC_METHODS(Player)

INIT_METHOD(Player)
	username = o["username"];
	password = o["password"];
	avatar = o["avatar"];
	details = Cache::find<Details>(o["details"]);
	leagues = decode<League>(o["leagues"]);
	friends = decode<Friend>(o["friends"]);
	invites = decode<Invite>(o["invites"]);
	cashiers = decode<Cashier>(o["cashiers"]);
	banks = decode<Bank>(o["banks"]);

	list<string> lm_kv = split('|',o["league_member_map"]);	
	list<string>::iterator i;
	for (i = lm_kv.begin(); i != lm_kv.end(); ++i) {
		if ((*i).empty()) continue;
		list<string> kv = split(',',(*i));
		string k = kv.front();
		kv.pop_front();
		string v = kv.front();
		league_member_map[Uint64_of_string(k)] = Cache::find<Member>(v);
	}

	return details != NULL;
END

TOS_METHOD(Player)
	string lmm = "";
	map<UInt64,Member*>::iterator i;
	for (i = league_member_map.begin(); i != league_member_map.end(); ++i) {
		if (lmm.empty()) {
			lmm = string_of_Uint64(i->first) + "," + (i->second == NULL ? "" : string_of_Uint64(i->second->id));
		} else {
			lmm = "|" + string_of_Uint64(i->first) + "," + (i->second == NULL ? "" :  string_of_Uint64(i->second->id));
		}
	}
	return "type=" + type() 
		+ "&username=" + username 
		+ "&avatar=" + avatar 
		+ "&password=" + password 
		+  "&details=" + ( details == NULL ? "" : string_of_Uint64(details->id))
		+ "&leagues=" + encode<League>(leagues) 
		+ "&friends=" + encode<Friend>(friends)
		+ "&invites=" + encode<Invite>(invites) 
		+ "&cashiers=" + encode<Cashier>(cashiers) 
		+ "&banks=" + encode<Bank>(banks) 
		+ "&league_member_map=" + lmm;
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
	return "type=" + type() 
		+ "&poster=" + (poster == NULL ? "" : string_of_Uint64(poster->id))
		+ "&parent=" + (parent == NULL ? "" : string_of_Uint64(parent->id))
		+ "&replies=" + encode<Post>(replies)
		+ "&subject=" + subject
		+ "&date=" + string_of_int(date)
		+ "&last=" + string_of_int(last)
		+ "&body=" + body;
END

string
Post::display()
{
	string reps;
	list<Post*>::iterator i;
	for (i = replies.begin(); i != replies.end(); ++i) {
		if (*i == NULL) continue;
		if (reps.empty()) {
			reps = string_of_Uint64((*i)->id);	
		} else {
			reps += "," + string_of_Uint64((*i)->id);	
		}
	}

	string retval = poster->username + "!" + 
		(parent == NULL ? "" : string_of_Uint64(parent->id)) + "!" +
		reps + "!" +
		subject + "!" +
		string_of_int(date) + "!" +
		string_of_int(last) + "!" +
		body;

	return retval;

}

// Replay Class

ALLOC_METHODS(Replay)

INIT_METHOD(Replay)
	string msgs = hexdecode(o["messages"]);
	messages = split('|',msgs);
END

TOS_METHOD(Replay)
	return "type=" + type() + "&messages=" + hexencode(join('|',messages));
END

bool
Replay::add(Message& m)
{
	cerr << "[Replay::add] adding data" << cerr;
//	Database* d = DBPool::grab();
//	d->begin();
	messages.push_back(m.build());
//	if (! save(d) || ! d->commit()) {
//		cerr << "[Replay::add] rollback commit failed" << cerr;
//		d->rollback();
//	}
//	DBPool::release(d);
	return true;
}

bool
Replay::send(Socket* s, int frame)
{
	bool retval = true;
	int j = 0;

	if (frame >= messages.size()) {
		s->login()->replay = NULL;
		s->login()->frame = 0;
		Message m;
		m.add("msg","replay");
		m.add("status","0");
		m.add("replay",string_of_Uint64(id));
		return s->send(m);
	}

	list<string>::iterator i;
	for (i = messages.begin(); i != messages.end(); ++i) {
		if ((*i).empty()) {
			cerr << "[Replay::send] empty message" << endl;
			continue;
		}
		if (j < frame) {
			++j;
			continue;
		}
		cerr << "[Replay::send] sending " << *i << endl;
		int out = s->out(*i);
		if ( (*i).size() > out) {
			cerr << "[Replay::send] " << out << "bytes sent, failed to send " << *i << endl;	
			retval = false;
		}
		++j;
		return retval;
	}
	return retval;
}

// HSP

HSP::HSP() {}
HSP::HSP(Player* a, const string& s) : p(a), h(s) {}
HSP::~HSP() {}

string
HSP::encode(list<HSP> l) {
	string retval;
	list<HSP>::iterator i;
	for (i = l.begin(); i != l.end(); ++i) {
		if (retval.empty()) {
			retval = ((*i).p == NULL ? "" : string_of_Uint64((*i).p->id)) + ":" + (*i).h;
		} else {
			retval += "|" + ((*i).p == NULL ? "" : string_of_Uint64((*i).p->id)) + ":" + (*i).h;
		}
	}
	return retval;
}

list<HSP>
HSP::decode(const string& s)
{
	list<HSP> retval;
	size_t i,o,j;
	o = 0;
	for (i = 0; i < s.size(); ++i) {
		if (s[i] != '|') continue;
		if (o < i) {
			for (j = o; j < i; ++j) {
				if (s[j] == ':') break;
			}	
			string key = s.substr(o,j-o);
			string value = s.substr(i,i-j);
			cerr << "HSP::decode] got " << key << " = " << value << endl;
			retval.push_back(HSP(
				(key.empty() ? NULL : Cache::find<Player>(key)),				value));
		}
		o = i+1;
	}
	if (o < i) {
		for (j = o; j < i; ++j) {
			if (s[j] == ':') break;
		}	
		string key = s.substr(o,j-o);
		string value = s.substr(i,i-j);
		cerr << "[HSP::decode] got " << key << " = " << value << endl;
		retval.push_back(HSP(
			(key.empty() ? NULL : Cache::find<Player>(key)),
			value));
	}
	return retval;
}

// PVP 

PVP::PVP() {}
PVP::PVP(Player* a, Player* b) : p(a), op(b) {};
PVP::~PVP() {}


string
PVP::encode(list<PVP> l) {
	string retval;
	list<PVP>::iterator i;
	for (i = l.begin(); i != l.end(); ++i) {
		if (retval.empty()) {
			retval = ((*i).p == NULL ? "" : string_of_Uint64((*i).p->id)) + ":" + ((*i).op == NULL ? "" : string_of_Uint64((*i).op->id));
		} else {
			retval += "|" + ((*i).p == NULL ? "" : string_of_Uint64((*i).p->id)) + ":" + ((*i).op == NULL ? "" : string_of_Uint64((*i).op->id));
		}
	}
	return retval;
}

list<PVP>
PVP::decode(const string& s)
{
	list<PVP> retval;
	size_t i,o,j;
	o = 0;
	for (i = 0; i < s.size(); ++i) {
		if (s[i] != '|') continue;
		if (o < i) {
			for (j = o; j < i; ++j) {
				if (s[j] == ':') break;
			}	
			string key = s.substr(o,j-o);
			string value = s.substr(i,i-j);
			cerr << "[PVP::decode] got " << key << " = " << value << endl;
			retval.push_back(PVP(
				(key.empty() ? NULL : Cache::find<Player>(key)),
				(value.empty() ? NULL : Cache::find<Player>(value))));
		}
		o = i+1;
	}
	if (o < i) {
		for (j = o; j < i; ++j) {
			if (s[j] == ':') break;
		}	
		string key = s.substr(o,j-o);
		string value = s.substr(i,i-j);
		cerr << "[PVP::decode] got " << key << " = " << value << endl;
		retval.push_back(PVP(
			(key.empty() ? NULL : Cache::find<Player>(key)),
			(value.empty() ? NULL : Cache::find<Player>(value))));
	}
	return retval;
}


// Season Class

ALLOC_METHODS(Season)

INIT_METHOD(Season)
	name = o["name"];
	state = static_cast<SeasonState>(int_of_string(o["state"]));
	events = decode<Event>(o["events"]);
	playoffs = decode<Event>(o["playoffs"]);
	has_playoffs = o["has_playoffs"];
	ranking = o["ranking"];
	qualifiers = o["qualifiers"];
	current = Cache::find<Event>(o["current"]);
	league = Cache::find<League>(o["league"]);
	teams = decode<Team>(o["teams"]);
	stats = decode<Player>(o["stats"]);
	holdem_stats = HSP::decode(o["holdem_stats"]);
	opp_stats = PVP::decode(o["opp_stats"]);
	posts = decode<Post>(o["posts"]);
	return true;
END

TOS_METHOD(Season)
	return "type=" + type() + "&name=" + name 
		+ "&events=" + encode<Event>(events) 
		+ "&playoffs=" + encode<Event>(playoffs)
		+ "&has_playoffs=" + has_playoffs
		+ "&ranking=" + ranking
		+ "&quaifiers=" + qualifiers
		+ "&current=" + (current == NULL ? "" : string_of_Uint64(current->id))
		+ "&league=" + (league == NULL ? "" :  string_of_Uint64(league->id))
		+ "&teams=" + encode<Team>(teams) 
		+ "&state=" + string_of_int(state)
		+ "&posts=" + encode<Post>(posts)
		+ "&stats=" + encode<Player>(stats)
		+ "&opp_stats=" + PVP::encode(opp_stats)
		+ "&holdem_stats=" + HSP::encode(holdem_stats);
END

bool Season::addEvents(list<Event*>& el)
{
	list<Event*>::iterator i,j;
	for (j = el.begin(); j != el.end(); ++j) {
		bool found = false;
		for (i = events.begin(); i != events.end(); ++i) {
			if ((*i)->date > (*j)->date) {
				events.insert(i,*j);
				found = true;
				break;
			}
		}
		if (! found) {
			events.push_back(*j);
		}
	}
}

bool Season::addPlayoffs(list<Event*>& el, string qual)
{
	if (! qual.empty()) {
		qualifiers = qual;
	}
	list<Event*>::iterator i,j;
	for (j = el.begin(); j != el.end(); ++j) {
		bool found = false;
		for (i = playoffs.begin(); i != playoffs.end(); ++i) {
			if ((*i)->date > (*j)->date) {
				playoffs.insert(i,*j);
				found = true;
				break;
			}
		}
		if (! found) {
			playoffs.push_back(*j);
		}
	}
}

bool Season::hasStats(Player* p)
{
	list<Player*>::iterator i;
	for (i = stats.begin(); i != stats.end(); ++i) {
		if (*i == p) return true;
	}
	return false;
}

bool Season::hasHoldemStats(Player* p, const string& h)
{
	list<HSP>::iterator i;
	for (i = holdem_stats.begin(); i != holdem_stats.end(); ++i) {
		if (i->p == p && i->h == h) return true;
	}
	return false;
}

bool Season::hasOppStats(Player* p, Player* o)
{
	list<PVP>::iterator i;
	for (i =  opp_stats.begin(); i != opp_stats.end(); ++i) {
		if ( i->p == p && i->op == o) return true;
	}
	return false;
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
	return "type=" + type() + "&name=" + name + "&ipaddr=" + ipaddr + "&port=" + string_of_int(port) + "&nat_port=" + string_of_int(nat_port);
END

// Table Class

ALLOC_METHODS(Table)

INIT_METHOD(Table)
	name = o["name"];
	event = Cache::find<Event>(o["event"]);
	games = decode<Game>(o["games"]);
	current = Cache::find<Game>(o["current"]);
	
	list<string> sts = split('|',o["seats"]);
	list<string>::iterator i;

	int v = 0;
	for (i = sts.begin(); i != sts.end(); ++i) {
		if ((*i).empty()) {
			if (v != MAX_SEATS) ++v;
			continue;
		}
		if (v != MAX_SEATS) {
			seats[v].decode(*i);	
			++v;
		}
	}
	
	return true;
END	

TOS_METHOD(Table)
	string sts;
	
	int s;
	for( s = 0; s != MAX_SEATS; ++s) {
		if (sts.empty()) {
			sts = seats[s].encode();
		} else {
			sts += "|" + seats[s].encode();
		}
	}
	
	return "type=" + type() 
		+ "&name=" + name 
		+ "&event=" + (event == NULL ? "" : string_of_Uint64(event->id)) 
		+ "&games=" + encode<Game>(games)
		+ "&current=" + (current == NULL ? "" : string_of_Uint64(current->id))
		+ "&seats=" + sts;
END

// Team Class

ALLOC_METHODS(Team)

INIT_METHOD(Team)
	name = o["name"];
	captain = Cache::find<Player>(o["captain"]);
	players = decode<Player>(o["players"]);
	points = int_of_string(o["points"]);
	position = int_of_string(o["position"]);
	return true;
END

TOS_METHOD(Team)
	return "type=" + type() + "&name=" + name + "&captain=" + (captain == NULL ? "" : string_of_Uint64(captain->id)) + "&players=" + encode<Player>(players) 
		+ "&points=" + string_of_int(points) + "&position=" + string_of_int(position);
END

// Tournament Calss

ALLOC_METHODS(Tournament)

INIT_METHOD(Tournament)
	cost = int_of_string(o["cost"]);
	chips = int_of_string(o["chips"]);
	rebuys.cost = int_of_string(o["rebuys.cost"]);
	rebuys.chips = int_of_string(o["rebuys.chips"]);
	rebuys.end_round = int_of_string(o["rebuys.end_round"]);
	rebuys.threshold= int_of_string(o["rebuys.threshold"]);
	addon.cost = int_of_string(o["addon.cost"]);
	addon.chips = int_of_string(o["addon.chips"]);
	list<string> rnds = split(',', o["rounds"]);
	list<string>::iterator i,j;
	for (i = rnds.begin(); i != rnds.end(); ++i) {
		list<string> fields = split('|',*i);
		struct round r;
		r.duration = int_of_string(fields.front());
		fields.pop_front();
		r.small_blind = int_of_string(fields.front());
		fields.pop_front();
		r.big_blind = int_of_string(fields.front());
		fields.pop_front();
		r.ante = int_of_string(fields.front());
		fields.pop_front();
		r.break_time = int_of_string(fields.front());
		rounds.push_back(r);
	}
END

TOS_METHOD(Tournament)
	string rnds; 
	list<struct round>::iterator i;
	for (i = rounds.begin(); i != rounds.end(); ++i) {
		if (rnds.empty()) {
			rnds = string_of_int(i->duration) + "|"
				+ string_of_int(i->small_blind) + "|"
				+ string_of_int(i->big_blind) + "|"
				+ string_of_int(i->ante) + "|"
				+ string_of_int(i->break_time);
		} else {
			rnds += "," + string_of_int(i->duration) + "|"
				+ string_of_int(i->small_blind) + "|"
				+ string_of_int(i->big_blind) + "|"
				+ string_of_int(i->ante) + "|"
				+ string_of_int(i->break_time);
		}
	}
	return "type=" + type() 
		+ "&cost=" + string_of_int(cost) 
		+ "&chips=" + string_of_int(chips)
		+ "&rebuys.cost=" + string_of_int(rebuys.cost)
		+ "&rebuys.chips=" + string_of_int(rebuys.chips)
		+ "&rebuys.end_round=" + string_of_int(rebuys.end_round)
		+ "&rebuys.threshold=" + string_of_int(rebuys.threshold)
		+ "&addon.cost=" + string_of_int(addon.cost)
		+ "&addon.chips=" + string_of_int(addon.chips)
		+ "&rounds=" + rnds;
END

tournament_seating::tournament_seating()
{
}

tournament_seating::~tournament_seating()
{
}

bool tournament_seating::operator==(const tournament_seating& ts)
{
	return ts.player == player &&  ts.table == table && seat == seat & purse == purse && position == position;
}

bool
Tournament::add_player(Player* p)
{
	list<tournament_seating>::iterator i;
	for (i = seating.begin(); i != seating.end(); ++i) {
		if (i->player == p) return false;
	}
	tournament_seating ts;
	ts.player = p;
	ts.table = NULL;
	ts.seat = -1;
	ts.purse = 0;
	ts.position = 0;
	seating.push_back(ts);
	return true;
}

bool
Tournament::remove_player(Player* p)
{
	list<tournament_seating>::iterator i;
	for (i = seating.begin(); i != seating.end(); ++i) {
		if ( i->player == p ) {
			tournament_seating ts = *i;
			seating.remove(ts);	
			return true;
		}
	}
	return false;
}

bool
Tournament::update_player(Player* p, Table* t, int seat, int chips, int position) 
{
	list<tournament_seating>::iterator i;
	for (i = seating.begin(); i != seating.end(); ++i ) {
		if ( i->player == p) {
			i->table = (t == NULL ? i->table: t);
			i->seat = (seat >= 0 ? seat : i->seat);
			i->purse = (chips >= 0 ? chips : i->purse);
			i->position = (position > 0 ? position : i->position);
			return true;
		}
	}
	return false;
}

bool
Tournament::seat_players(list<Table*>& t)
{
	int steps, k, count, seat;
	list<Table*>::iterator j;	
	list<tournament_seating>::iterator i;
	count = seating.size();
	cerr << "[Tournament::seat_players] count is: " << count << endl;
	cerr << "[Tournament::seat_players] players per table is: " << (count / t.size()) << endl;
	seat = 0;
	struct tmp_seating {
		Table* t;
		int seat;
		struct tmp_seating* next;	
	} tseating[ count ], *ts;
	seat = 0;
	for (int k = 0; k < count; ++seat ) {
		for (j = t.begin(); j != t.end(); ++j) {
			cerr << k << " seating " << seat << " at " << *j << endl;
			tseating[k].next = &tseating[k+1];
			tseating[k].t = *j;
			tseating[k].seat = seat;
			++k;
			if (k >= count) {
				cerr << "k out of range " << k << " vs " << count << endl;
				break;
			}
		}
	}
	tseating[count-1].next = &tseating[0];
	ts = &tseating[ rand() % count ];
	cerr << "Begin Seating " << endl;
	for (i = seating.begin(); i != seating.end(); ++i) {
		cerr << "Next Seat " << ts->next->seat << " this " << ts->seat << endl;
		cerr << "Next Table " << ts->next->t << " this " << ts->t << endl;

reseat:
		Database* d = DBPool::grab();
		d->begin();
		Cashier* c = Cashier::alloc(d);
		Event* e = i->table->event;

		c->player = i->player;
		c->event = e;
		c->pool = (e->options->value == 0 ? Cashier::FREEBIE : (e->season == NULL ? Cashier::LEAGUE : Cashier::SEASON));
		c->kind = Cashier::TOURNEY;
		c->chips = cost;
		i->player->cashiers.push_back(c);
		e->cashiers.push_back(c);

		
		i->purse = chips;
		i->table = ts->next->t;
		i->seat = ts->next->seat;
		i->table->seats[i->seat].player = i->player;
		i->table->seats[i->seat].seat = i->seat;
		cerr << "Seating player " << i->player->username << " in seat " << i->seat << " at table " << i->table << endl;
		ts->next = ts->next->next;

		if (! c->save(d) || ! i->player->save(d) || ! e->save(d) || ! i->table->save(d) || ! d->commit()) {
			i->player->cashiers.pop_back();
			e->cashiers.pop_back();
			d->rollback();
			DBPool::release(d);
			goto reseat;
		}
		DBPool::release(d);
		for (k = 0; k < rand() % --count; ++k) {
			ts = ts->next;
		}
	}
	cerr << "Seating done " << endl;
	cerr << "[Tournament::seat_players] done" << endl;
	return true;
}

string
Tournament::get_seating()
{
	string retval;
	list<tournament_seating>::iterator i;
	for (i = seating.begin(); i != seating.end(); ++i) {
		if (i->player == NULL) continue;
		if (retval.empty()) {
			retval = i->player->username + "," 
				+ (i->table == NULL ? "" : string_of_Uint64(i->table->id)) + ","
				+ string_of_int(i->seat) + ","
				+ string_of_int(i->purse) + ","
				+ string_of_int(i->position);
		} else {
			retval += "!" + i->player->username + "," 
				+ (i->table == NULL ? "" : string_of_Uint64(i->table->id)) + ","
				+ string_of_int(i->seat) + ","
				+ string_of_int(i->purse) + ","
				+ string_of_int(i->position);

		}
	}
	return retval;
}

bool
Tournament::fabricate_rounds()
{
	int small, big, ante, duration;	
	list<struct round>::iterator i;
	for ( i = rounds.begin(); i != rounds.end(); ++i) {
		if (small < i->small_blind) small = i->small_blind;
		if (big < i->big_blind) big = i->big_blind;
		if (ante < i->ante) ante = i->ante;
		duration = i->duration;	
	}
	int max_money = chips * seating.size();
	int j = 2;
	while ( j * (big + ante) < max_money ) {
		struct round r;
		r.duration = duration;
		r.break_time = 0;
		r.small_blind = small * j;
		r.big_blind = big * j;
		r.ante = ante * j;
		rounds.push_back(r);
		j *= 2;
	}
	return true;
}

bool
Tournament::is_registered(Player* p)
{
	list<tournament_seating>::iterator i;
	for (i = seating.begin(); i != seating.end(); ++i) {
		if (i->player == NULL) continue;
		if (i->player == p ) return true;
	}
	return false;
}

// Cache Class

Cache* Cache::cache = NULL;

Cache::Cache() {};
Cache::~Cache() {};

bool 
Cache::create(const string& s)
{
	if (cache != NULL) return false;

	cache = new Cache();
	
	string load_cache = "SELECT value FROM cache_table WHERE id = " + s;
	Database* d = DBPool::grab();
	d->begin();
	Result* res = d->query(load_cache);

	if (res == NULL || ! res->success() || res->rows != 1) {
		cerr << "[Cache::init] failed " << d->error() << endl;
		cache->next_guid = Uint64_of_string(s);
		cache->idspace = Uint64_of_string(s);
		cache->id = Indexable::create(d);
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
			stats = "&" + i->first + "=" + string_of_int(i->second);
		} else {
			stats += "&" + i->first + "=" + string_of_int(i->second);
		}
	}
	return "type=" + type()  
		+ "&idspace=" + string_of_Uint64(idspace)
		+ "&next_guid=" + string_of_Uint64(next_guid)
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
