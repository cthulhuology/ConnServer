// mail_modules.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//

#include "mail_modules.h"
#include <time.h>
#include <ctype.h>
#include <iostream>
#include "dir.h"
#include "env.h"
#include "database.h"
#include "data.h"
#include "esocket.h"

BEGIN_REGISTRY
REGISTER(send-replay,SendReplayModule)
REGISTER(send-invite,SendInviteModule)
REGISTER(send-password,SendPasswordModule)
END_REGISTRY

string replace_vars(const string& source, map<string,string>& vars) 
{
	string retval = "";
	size_t i,o;
	o = 0;
	for (i = 0; i < source.size(); ++i) {
		if (source[i] == '$') {
			for (o = i+1; i < source.size() && !isspace(source[i]); ++i);
			string var = source.substr(o,i-o);
			cerr << "found var [" << var << "]" << endl;
			retval += hexdecode(vars[var]) + source[i];	
		} else {
			retval += source[i];
		}
	}
	return retval;
}

bool mail_string(const string& email, const string& body)
{
	Socket* s = Socket::create();
	if (! s->conn("mail.ihomegame.com","25"))  {
		cerr << "Failed to connect to local host" << endl;
		Socket::finalize(s);
		return false;
	}

	string m = "HELO mail.ihomegame.com\r\nMAIL FROM: commish@ihomegame.com\r\nRCPT TO: " + email + "\r\nDATA\r\n";

	cerr << "SENDING [" << m << body << "\r\n.\r\n]" << endl;
	bool retval = s->out(m + body + "\r\n.\r\n");
	return retval;
}

string load_template(const string& filename, bool force)
{
	static map<string,string> file_cache;
	string path = Env::env["CSROOT"];
	if (path.empty()) {
		cerr << "load_template failed, no env variable CSROOT" << endl;
		return "";
	}

	if (! file_cache[filename].empty() && ! force) {
		return file_cache[filename];
	}

	Directory dir(path);
	if (! dir.exists(filename)) {
		cerr << "load_template failed, so such file " << filename << endl;	
		return "";
	}

	int fd = dir[filename];
	string text = "";
	char c;
	while(read(fd,&c,1) > 0) {
		text += c;
	}
	file_cache[filename] = text;
	return text;
}

bool send_invite(Invite* invite, const string& temp)
{
	if (invite == NULL) {
		cerr << "send_invite failed, recieved null pointer" << endl;
		return false;
	}

	map<string,string> variables;	
	variables["INVITE"] = string_of_Uint64(invite->id);
	variables["SENDER"] = invite->sender->details->name;
	switch(invite->kind) {
		case Invite::LEAGUE:
			variables["LEAGUE"] = (invite->target.league == NULL ? "" : invite->target.league->name);
			break;
	}
	switch(invite->dest) {
		case Invite::EMAIL:
			variables["EMAIL"] = "<" + invite->recipient.email.address + ">";
			variables["NAME"] = invite->recipient.email.name;
			break;
	}
	variables["MESSAGE"] = invite->message;

	if (temp.empty()) {
		cerr << "send_invite failed, no template file found" << endl;
		return false;
	}

	string body = replace_vars(load_template(temp,false),variables);

	return mail_string(hexdecode(variables["EMAIL"]),body);
}

MODULE_PROCESS(SendInviteModule)
{
	LOGIN_CHECK
	MSG_CHECK("invite,template")

	Invite* invite = Cache::find<Invite>(m["invite"]);
	return send_invite(invite,m["template"]);		
}

MODULE_PROCESS(SendReplayModule)
{
	LOGIN_CHECK
	MSG_CHECK("replay,email,name");

	Replay* replay = Cache::find<Replay>(m["replay"]);
	Player* player = c->login()->player;

	if (replay == NULL || player == NULL)  {
		cerr << "[SendReplayModule] invalid replay or player " << endl;
		m.add("status","1");
		return c->send(m);;
	}
	
	map<string,string> variables;
	variables["REPLAY"] = string_of_Uint64(replay->id);
	variables["SENDER"] = player->details->name;
	variables["NAME"] = m["name"];
	variables["MESSAGE"] = m["message"];
	variables["EMAIL"] = "<" + m["email"] + ">";

	string body = replace_vars(load_template("replay.txt",false),variables);
	if (mail_string(hexdecode(variables["EMAIL"]),body)) {
		m.add("status","0");
	} else {
		m.add("status","1");
	}
	return c->send(m);
}

MODULE_PROCESS(SendPasswordModule)
{
	MSG_CHECK("username");

	Database* d = DBPool::grab();
	d->begin();

	string query = "SELECT password, player_id FROM login WHERE username = '" + m["username"] + "'";
	cerr << "[SendPasswordModule] " << query << endl;
	
	Result* res = d->query(query);
	if (res == NULL || res->rows == 0) {
		delete res;
		d->rollback();
		m.add("status","1");
		DBPool::release(d);
		return c->send(m);
	}
	
	Player* p = Cache::find<Player>((*res)["player_id"][0]);
	if (p->details == NULL) {
		delete res;
		d->rollback();
		m.add("status","1");
		DBPool::release(d);
		return c->send(m);
	}

	map<string,string> variables;
	variables["PASSWORD"] = (*res)["password"][0];
	variables["NAME"] = m["username"];
	variables["EMAIL"] = "<" + m["email"] + ">";

	string body = replace_vars(load_template("password.txt",false),variables);
	if (mail_string(hexdecode(variables["EMAIL"]),body)) {
		m.add("status","0");
	} else {
		m.add("status","1");
	}
	return c->send(m);
}
