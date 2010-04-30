// module.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// This is the parent class of all message handlers
//

#ifndef __HAVE_MODULES__
#define __HAVE_MODULES__

#include "message.h"
#include "esocket.h"

#define VERSTR2(x,y) #x "_" #y
#define VERSTR(x,y) VERSTR2(x,y)

#define MODSYM(x,y) x ## _ ## y
#define MODNAME2(x,y) MODSYM(x,y)
#define MODCONST2(x,y) MODNAME2(x,y)::MODNAME2(x,y)
#define MODDEST2(x,y) MODNAME2(x,y)::~MODNAME2(x,y)
#define MODCONST(x) MODCONST2(x,VERSION)()
#define MODDEST(x) MODDEST2(x,VERSION)()
#define MODNAME(x) MODNAME2(x,VERSION)
#define MODNAMEDISP(x) string MODNAME(x)::name() { return VERSTR(x,VERSION); }

#define MODULE(x) class MODNAME(x) : public Module { public: MODNAME(x)(); ~MODNAME(x)(); bool process(Socket* c, Message& m); string name(); };

#define DEFAULT_MODULE(x) MODCONST(x) {} MODDEST(x) {} MODNAMEDISP(x)
#define PROCESS(x) bool MODNAME(x)::process(Socket* c, Message& m)
#define MODULE_PROCESS(x) DEFAULT_MODULE(x) PROCESS(x)

#define BEGIN_REGISTRY extern "C" {	\
	void register_module(Registry* r) {
#define REGISTER(x,y) r->associate(new MODNAME(y)(), #x);
#define END_REGISTRY }}

class Registry;
class Module {
	public:
		virtual bool process(Socket* c, Message& m) = 0;
		virtual string name() = 0;
		Module();
		~Module();
		static bool call(const string& s, Socket* c, Message& m);
};


class Registry {
	public:
		static bool init();
		static bool finalize();
		static bool load();
		static bool check(const string& f);
		static Module* find(const string& s); 
		static void dump();
		bool associate(Module* m, const string& msg);
	protected:
		Registry();
		~Registry();
	private:
		static Registry* reg;
		map<string,Module*> mods;
		vector<void*> libptrs;
		vector<string> libraries;
};

#endif
