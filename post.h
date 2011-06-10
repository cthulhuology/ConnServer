// Post Class
//
// © 2010 David J. Goehrig
//

#ifndef __HAVE_POST_H__
#define __HAVE_POST_H__

#include "player.h"

class Post : public Object {
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

#endif
