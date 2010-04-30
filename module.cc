// module.cc
//
// Copyright (C) 2004,2005 David J. Goehrig
// All rights reserved
//
// Parent class of message handlers
//

#include <dlfcn.h>
#include "module.h"
#include "env.h"
#include "dir.h"

#ifdef MACOSX
#define MODEXT ".dylib"
#else
#define MODEXT ".so"
#endif

// Module 
//

Module::Module()
{
}

Module::~Module()
{
}

bool Module::call(const string& s, Socket* c, Message& m)
{
	Module* mod = Registry::find(s);
	if (mod == NULL) return false;
	return mod->process(c,m);
}

// Registry
//
// Every module must be included in the Registry constructor, this way every registered feature
// will be initialized in the array.  We vector off this array to provide processing.
//

Registry* Registry::reg = NULL;

bool Registry::init()
{
	if (reg != NULL) {
		cerr << "Registry already initialized" << endl;
		return true;
	}

	reg = new Registry();
	return Registry::load();
}

bool Registry::finalize()
{
	if (reg == NULL) {
		cerr << "Registry already finalized" << endl;
		return false;
	}
	delete reg;
	reg = NULL;
	return true;
}

Module* Registry::find(const string& s)
{
	return reg->mods[s];
}

void Registry::dump()
{
	vector<string>::iterator j;
	cerr << "Libraries:" << endl;
	for (j = reg->libraries.begin(); j != reg->libraries.end(); ++j) {
		cerr << "\t" << (*j) << endl;
	}
	map<string,Module*>::iterator i;
	cerr << "Registry:" << endl;
	for (i = reg->mods.begin(); i!= reg->mods.end(); ++i) {
		cerr << "\t" << (*i).first << " -> " << (*i).second->name() << endl;
	}
}

bool Registry::associate(Module* m, const string& msg)
{
	cerr << "Registry::associate registering message " << msg << endl;
	mods[msg] = m;
}

bool Registry::check(const string& s)
{
	vector<string>::iterator i;
	for (i = reg->libraries.begin(); i != reg->libraries.end(); ++i) {
		if (s == *i) return true;
	}
	return false;
}

bool Registry::load()
{
	if (Env::env["CSMODULEHOME"].empty()) {
		cerr << "Registry::init invalid CSMODULEHOME environ variable" << endl;
		return false;
	}
	Directory dir(Env::env["CSMODULEHOME"]);
	vector<string> v = dir.files();

	vector<string>::iterator i;
	for (i = v.begin(); i != v.end(); ++i) {
		cerr << "attempting to load " << *i << " modex=" << MODEXT << " sizeof modex=" << sizeof(MODEXT) << endl;
		if ((*i).substr((*i).length()-(sizeof(MODEXT)-1)) != MODEXT) continue;
		cerr << "Loading module " << *i << endl;
		if (! check(*i)) {
			reg->libraries.push_back(*i);
			void* libptr = dlopen((*i).c_str(), RTLD_NOW);
			if (libptr  == NULL) {
				cerr << "Registry::Registry failed to load " << *i <<  " : " << dlerror() << endl;
				continue;
			} 
			void (*r)(Registry*) = (void (*)(Registry*))dlsym(libptr,"register_module");
			(*r)(reg);
			reg->libptrs.push_back(libptr);
		}
	}
	return true;
}

Registry::Registry()
{
}

Registry::~Registry()
{
	map<string,Module*>::iterator i;
	for (i = mods.begin(); i != mods.end(); ++i) {
		delete (*i).second;
	} 
	vector<void*>::iterator j;
	for (j = libptrs.begin(); j != libptrs.end(); ++j) {
		dlclose(*j);	
	}
}

#ifdef TEST_CASE

int
main(int argc, char** argv)
{
	if(! Registry::init()) {
		cerr << "Failed to initialize registry" << endl;
		exit(1);
	}
	reg->dump();	
	if(! Registry::finalize()) {
		cerr << "Failed to finalize registry" << endl;
		exit(1);
	}
	return 0;
}

#endif
