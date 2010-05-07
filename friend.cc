// Friend Class
//
// © 2010 David J. Goehrig
//

#include "friend.h"

ALLOC_METHODS(Friend)

INIT_METHOD(Friend)
	OBJ_PROPERTY(player,Player);
	PROPERTY(category);
	PROPERTY(rating);
	PROPERTY(personality);
	PROPERTY(notes);
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

