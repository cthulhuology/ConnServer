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
		+ "\nposter:" + (poster == NULL ? "" : string_of_Uint64(poster->id))
		+ "\nparent:" + (parent == NULL ? "" : string_of_Uint64(parent->id))
		+ "\nreplies:" + encode<Post>(replies)
		+ "\nsubject:" + subject
		+ "\ndate:" + string_of_int(date)
		+ "\nlast:" + string_of_int(last)
		+ "\nbody:" + body;
END

string Post::display() {
	return "{ " + body + " }";
}

