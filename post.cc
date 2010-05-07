// Post Class
//
// © 2010 David J. Goehrig
//

#include "post.h"

ALLOC_METHODS(Post)

INIT_METHOD(Post)
	OBJ_PROPERTY(poster,Player);
	OBJ_PROPERTY(parent,Post);
	LIST_PROPERTY(replies,Post);
	PROPERTY(subject);
	INT_PROPERTY(date);
	INT_PROPERTY(last);
	PROPERTY(body);
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

