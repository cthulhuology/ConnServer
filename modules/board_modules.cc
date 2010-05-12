// board_modules.cc
//
//	Copyright (C) 2005 David J. Goehrig
//	All Rights Reserved
//

#include "board_modules.h"
#include "database.h"
#include "object.h"
#include "post.h"
#include "login.h"

BEGIN_REGISTRY
REGISTER(add-post,AddPostModule)
REGISTER(remove-post,RemovePostModule)
REGISTER(posts,PostsModule)
END_REGISTRY

MODULE_PROCESS(AddPostModule)
{
	MSG_CHECK("parent,subject,date,body");
	LOGIN_CHECK

	Database* d = DBPool::grab();
	d->begin();
	
	Post* post = Post::alloc(d);
	post->poster = c->login()->player;
	post->date = int_of_string(m["date"]);
	post->parent = Cache::find<Post>(m["parent"]);
	if (post->parent != NULL) {
		post->parent->replies.push_back(post);
		post->parent->last = post->date;
	}
	post->subject = m["subject"];
	
	post->body = m["body"];

	if (! post->save(d) || ! d->commit()) {
		if (post->parent != NULL) {
			post->parent->replies.pop_back();
		}
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(RemovePostModule)
{
	MSG_CHECK("post");
	LOGIN_CHECK
	
	Post* post = Cache::find<Post>(m["post"]);
	if (post == NULL) {
		m.add("status","1");
		return c->send(m);
	}

	Post* parent = post->parent;
	parent->replies.remove(post);
	
	Database* d = DBPool::grab();
	d->begin();
	if (! parent->save(d) || ! d->commit()) {
		parent->replies.push_back(post);
		d->rollback();
		DBPool::release(d);
		m.add("status","1");
		return c->send(m);
	}	
	DBPool::release(d);
	m.add("status","0");
	return c->send(m);
}

MODULE_PROCESS(PostsModule)
{
	if (m["posts"].empty() && m["season"].empty() && m["league"].empty()) {
		m.add("status","1");
		c->send(m);
	}
	LOGIN_CHECK

	string replies;
	list<Post*>::iterator i;
	if (! m["post"].empty()) {
		Post* post = Cache::find<Post>(m["post"]);
		if (post == NULL) {
			m.add("status","1");
			return c->send(m);
		}	
		for(i = post->replies.begin(); i != post->replies.end(); ++i) {
			if (*i == NULL) continue;
			m.add(string_of_Uint64((*i)->id),(*i)->display());
		}
		m.add(string_of_Uint64(post->id),post->display());
	}

	m.add("status","0");
	return c->send(m);
}

