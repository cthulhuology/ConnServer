// player_modules.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved

#include <list>
#include "player_modules.h"
#include "database.h"
#include "user_map.h"
#include "server.h"
#include "util.h"
#include "object.h"
#include "player.h"
#include "login.h"
#include "details.h"

// Login Module
// 	for loging users into the system

BEGIN_REGISTRY
REGISTER(login,LoginModule)
REGISTER(register,RegisterModule)
REGISTER(disconnect,DisconnectModule)
REGISTER(getdetails,GetDetailsModule)
REGISTER(setdetails,SetDetailsModule)
REGISTER(quit,QuitModule)
REGISTER(setavatar,SetAvatarModule)
REGISTER(getavatar,GetAvatarModule)
END_REGISTRY

MODULE_PROCESS(LoginModule)
{
	list<string> ur = split('@',m["player"]);
	if ( ur.size() != 2) {
		m.add("status","1");
		return c->send(m);
	}
	string un= ur.front();
	ur.pop_front();
	string rm = ur.front();
	cerr << "Username: " << un << " connecting to room " << rm << endl;

	string lq= "SELECT password, player_id FROM login WHERE username = '" + un + "'";
	Database* d = DBPool::grab();
	d->begin();
	Result* res = d->query(lq);
	if ( res == NULL || ! res->success() ) {
		cerr << "LoginModule failed " << d->error() << endl;
		m.add("status","invalid user");
		delete res;
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	} 
	if (res->rows != 1) {
		cerr << "LoginModule message bad username " << m.dump() << endl;
		m.add("status","1");
		delete res;
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	}
	if ( m["pw"] != (*res)["password"][0] ) { 
		cerr << "LoginModule bad password " << m.dump() << endl;
		m.add("status","2");
		delete res;
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	}
	Login* login = Login::alloc(d);
	login->player = Cache::find<Player>((*res)["player_id"][0]);
	login->server = Server::cs();
	login->sock = c;
	c->set_login(login);
	UserMap::add(login);
	if ( rm != "lobby" ) {
		Room* room = Server::room(rm);
		if (room != NULL) {
			room->add(c);
			room->announce("announce",m["player"],login->player->avatar,"0");
		} else {
			m.add("status","3");
		}
	} else {
		Message msg;
		msg.add("msg","notify");
		msg.add("un",login->player->username);
		msg.add("online","1");
		c->set_room(NULL);
	}
	if (! d->commit()) {
		d->rollback();
		m.add("status","1");
	}
	if (m["status"].empty()) {
		m.add("status","0");
	}
	delete res;
	DBPool::release(d);
	return c->send(m);
}

// RegisterModule
//	for registering new users

MODULE_PROCESS(RegisterModule)
{
	int err;
	Result* res;
	Database* d = DBPool::grab();
	d->begin();

	string un = m["un"];
	string existing_query = "SELECT player_id FROM login WHERE username = '" + un + "'";
	res = d->query(existing_query);
	if (res == NULL || ! res->success() || res->rows) { 
		if (res == NULL || ! res->success()) {
			cerr << "RegisterModule::process failed " << d->error() << endl;
			m.add("status","1");
		}
		if (res->rows) {
			cerr << "RegisterModule::process failed username already taken" << endl;
			m.add("status","2");
		}
		d->rollback();
		DBPool::release(d);
		delete res;
		return c->send(m);
	}
	delete res;

	Player* player = Player::alloc(d);
	player->username =  un;
	player->password = m["pw"];
	player->avatar = "";
	player->details = Details::alloc(d);
	player->details->name = m["fname"] + " " + m["lname"];
	player->details->email = m["email"];
	player->details->phone = m["phone"];	
	player->details->cell = m["cell"];	
	player->details->street = m["address1"];	
	player->details->apartment = m["address2"];	
	player->details->city = m["city"];
	player->details->state = m["state"];
	player->details->zip = m["zip"];
	player->details->country = m["country"];
	player->details->dob = m["dob"];
	player->details->sex = m["sex"];
	player->details->occupation = m["occupation"];
	player->details->spam = m["spam"];

	if (! player->save(d) || ! player->details->save(d)) {
		cerr << "RegisterModule failed to save player" << endl;
		m.add("status","1");
		delete res;
		d->rollback();
		DBPool::release(d);
		return c->send(m);
	}

	string login_insert =  "INSERT INTO login (username,password,player_id) VALUES ('" + player->username + "','" + player->password + "'," + string_of_Uint64(player->id) + ")";
	
	res = d->query(login_insert);
	if (res == NULL || ! res->success())  {
		cerr << "RegisterModule::process failed " << d->error() << endl;
		cerr << "RegisterModule::process message: " << m.dump() << endl;
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
		m.add("player", un + "@lobby");
		d->commit();	
	}
	delete res;
	DBPool::release(d);
	return c->send(m);
}


// Get Details Module
// 	for returning a user's registered personal info


MODULE_PROCESS(GetDetailsModule)
{
	LOGIN_CHECK
	Login* login = c->login();

	if (login->player->details == NULL) {
		cerr << "GetDetailsModule invalid player w/ details" << endl;
		return false;
	}

	string fname,lname;
	list<string> flname = split(' ', login->player->details->name);
	fname = flname.front();
	flname.pop_front();
	lname = flname.front();

	m.add("status","0");
	m.add("fname",fname);
	m.add("lname",lname);
	m.add("email",login->player->details->email);
	m.add("phone",login->player->details->phone);
	m.add("cell",login->player->details->cell);
	m.add("address1",login->player->details->street);
	m.add("address2",login->player->details->apartment);
	m.add("state",login->player->details->state);
	m.add("city",login->player->details->city);
	m.add("zip",login->player->details->zip);
	m.add("country",login->player->details->country);
	m.add("dob",login->player->details->dob);
	m.add("sex",login->player->details->sex);
	m.add("occupation",login->player->details->occupation);
	m.add("income",login->player->details->income);
	m.add("spam",login->player->details->spam);
	return c->send(m);
}

// Set Details Module
// 	for updating the user's registered personal information


MODULE_PROCESS(SetDetailsModule)
{
	LOGIN_CHECK
	Login* login = c->login();

	if (login == NULL || login->player == NULL || login->player->details == NULL) {
		cerr << "SetDetailsModule invalid player w/ details" << endl;
		return false;
	}

	login->player->details->name = m["fname"] + " " + m["lname"];
	login->player->details->email = m["email"];
	login->player->details->cell = m["cell"];
	login->player->details->phone = m["phone"];
	login->player->details->street = m["address1"];
	login->player->details->apartment = m["address2"];
	login->player->details->city = m["city"];
	login->player->details->state = m["state"];
	login->player->details->zip = m["zip"];
	login->player->details->country = m["country"];
	login->player->details->dob = m["dob"];
	login->player->details->sex = m["sex"];
	login->player->details->occupation = m["occupation"];
	login->player->details->income = m["income"];
	login->player->details->spam = m["spam"];

	Database* d = DBPool::grab();
	if (! login->player->details->save(d)) {
		d->rollback();
		m.add("status","1");
	} else {
		d->commit();
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(DisconnectModule)
{
	Login* login = c->login();
	if (login != NULL) {
		if (c->room() != Server::room("lobby")) {
			c->room()->announce("disconnecting",string_of_Uint64(login->player->id),"","0");
		} else {
			Message msg;
			msg.add("msg","notify");
			msg.add("un",login->player->username);
			msg.add("online","0");
		}
	}
	return true;
}

MODULE_PROCESS(QuitModule)
{
	return false;
}

MODULE_PROCESS(SetAvatarModule)
{
	LOGIN_CHECK
	MSG_CHECK("avatar")

	Login* login = c->login();
	string oldavatar = login->player->avatar;
	login->player->avatar = m["avatar"];
	Database* d = DBPool::grab();
	d->begin();
	if (! login->player->save(d) || ! d->commit()) {
		login->player->avatar = oldavatar;
		d->rollback();
		m.add("status","1");
	} else {
		m.add("status","0");
	}
	DBPool::release(d);
	return c->send(m);
}

MODULE_PROCESS(GetAvatarModule)
{
	LOGIN_CHECK
	Login* login = c->login();
	if (login == NULL || login->player == NULL) {
		m.add("status","1");
		return c->send(m);
	}
	
	m.add("avatar",login->player->avatar);
	m.add("status","0");
	return c->send(m);
}

