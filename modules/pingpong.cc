// pingpong.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
// 	For testing connections

#include "pingpong.h"
#include "util.h"

BEGIN_REGISTRY
REGISTER(ping,PingModule)
REGISTER(pong,PongModule)
REGISTER(getcurrenthour,GetCurrentHourModule)
REGISTER(time,TimeModule)
END_REGISTRY


MODULE_PROCESS(PingModule)
{
	return c->send(m);
}

MODULE_PROCESS(PongModule)
{
	return true;
}

MODULE_PROCESS(GetCurrentHourModule)
{
	Message msg;
	struct tm *tmp;
	time_t t;

	t = time(NULL);
	tmp = localtime(&t);

	msg.add("msg","getCurrentHour").add("hour",string_of_int(tmp->tm_hour)).add("status","0");

	return c->send(msg);
}

MODULE_PROCESS(TimeModule)
{
	time_t t = time(NULL);
	m.add("time",string_of_int(t)).add("status","0");
	return c->send(m);
}
