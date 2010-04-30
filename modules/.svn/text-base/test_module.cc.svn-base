// test_module.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// A module class that does nothing

#include "test_module.h"
#define STR(x) #x 
#define XVERSTR(x) STR(x)

BEGIN_REGISTRY
REGISTER(test,TestModule)
END_REGISTRY

MODULE_PROCESS(TestModule)
{
	cerr << "Build Version= " XVERSTR(VERSION) << endl;
	Registry::dump();
	return true;
}

