// interp.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// 	embedding python into the CS so we can expand using scripts
//	and do our configuration stuff with python scripts

#include "interp.h"
#include <Python.h>

Interpreter* Interpreter::interp = NULL;

Interpreter::Interpreter(int argc, char** argv)
{
	Py_Initialize();	
	PySys_SetArgv(argc,argv);
}

Interpreter::~Interpreter()
{
	Py_Finalize();
}

void Interpreter::init(int argc, char** argv)
{
	if (interp == NULL) {
		interp = new Interpreter(argc,argv);
	}
}

void Interpreter::finalize()
{
	if (interp == NULL) return;
	delete interp;
	interp = NULL;
}
