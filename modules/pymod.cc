// pymod.cc
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// python based modules for chat server
//

#include "pymod.h"
#include "env.h"
#include "dir.h"
#include "user_map.h"
#include "server.h"

BEGIN_REGISTRY
PyModule::register_pymodules(r);
END_REGISTRY

PyModule::PyModule(const string& s)
{
	PyObject *name, *dict;
	module = NULL;
	callable = NULL;
	message = "";

	name = PyString_FromString(s.c_str());
	if (name == NULL) {
		cerr << "PyModule::PyModule could not import module " << s << endl;
		return;
	}

	module = PyImport_Import(name);
	Py_DECREF(name);
	
	dict = PyModule_GetDict(module);
	if (dict == NULL) {
		cerr << "PyModule::PyModule could not find dict for " << s << endl;
		return;
	}
	
	PyObject* msg = PyDict_GetItemString(dict,"message");
	if (msg == NULL) {
		cerr << "PyModule::PyModule no message associated with " << s << endl;
		return;
	}
	message = PyString_AsString(msg);
	if ( message.empty() || message[0] == '\0') {
		cerr << "PyModule::PyModule invalid message associated with " << s << endl;
		return;
	}

	callable = PyDict_GetItemString(dict,"process");
	if (callable == NULL || !PyCallable_Check(callable)) {
		cerr << "PyModule::PyModule no process associated with " << s << endl;
		return;
	}
}

PyModule::~PyModule()
{
	Py_DECREF(module);
}

string PyModule::name()
{
	return "PyModule";
}

bool PyModule::process(Socket* c, Message& m)
{
	if (module == NULL || callable == NULL) return false;	
	PyObject *args, *value;
	
	int i = 0;
	args = PyTuple_New(3);
	PyTuple_SetItem(args,i,PyCSServer_New());
	PyTuple_SetItem(args,++i,PyCSSocket_New(c));
	PyTuple_SetItem(args,++i,PyCSMessage_New(m));
	
	value = PyObject_CallObject(callable,args);

	if (value == NULL || value == Py_False) return false;	

	Py_DECREF(value);
	Py_DECREF(args);
	return true;
}

void PyModule::register_pymodules(Registry* r)
{
	if (Env::env["CSPYMODULEHOME"].empty()) {
		cerr << "PyModule::register_pymodules failed, please set CSPYMODULEHOME" << endl;
		return;
	}

	Directory pymoddir(Env::env["CSPYMODULEHOME"]);
	
	vector<string> files = pymoddir.files();
	vector<string>::iterator i;

	for (i = files.begin(); i != files.end(); ++i) {
		PyModule* mod = new PyModule(*i);
		if (mod == NULL 
		|| mod->message.empty()
		|| mod->callable == NULL) {
			cerr << "PyModule::register_pymodules failed to load " << *i << endl;
			continue;
		}
		r->associate(mod,mod->message);
	}
}

extern PyTypeObject cs_ServerType;
extern PyTypeObject cs_SocketType;
extern PyTypeObject cs_RoomType;

// Python Module init

#ifndef PyMODINIT_FUNC
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initcs(void)
{
	PyObject* mod;

	cs_ServerType.tp_new = PyType_GenericNew;
	cs_SocketType.tp_new = PyType_GenericNew;
	cs_RoomType.tp_new = PyType_GenericNew;

	if (PyType_Ready(&cs_ServerType) < 0) return;
	if (PyType_Ready(&cs_SocketType) < 0) return;
	if (PyType_Ready(&cs_RoomType) < 0) return;
	
	mod = Py_InitModule3("cs",cs_methods,"CS module");

	Py_INCREF(&cs_ServerType);
	Py_INCREF(&cs_SocketType);
	Py_INCREF(&cs_RoomType);

	PyModule_AddObject(mod, "Server", (PyObject*)&cs_ServerType);
	PyModule_AddObject(mod, "Socket", (PyObject*)&cs_SocketType);
	PyModule_AddObject(mod, "Room",(PyObject*)&cs_RoomType);
}


// cs_RoomObject Methods
//

void
cs_RoomObject_dealloc(cs_RoomObject* self)
{
	// don't delete the room because the server probably owns a reference
	// if we get the room object
}

PyObject*
cs_RoomObject_finalize(cs_RoomObject* self)
{
	if (self->room) {
		delete self->room;
	}
}

PyObject*
cs_RoomObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	cs_RoomObject* self;
	self = (cs_RoomObject*)type->tp_alloc(type,0);
	if (self == NULL) return (PyObject*)NULL;
	self->room = NULL;
	return (PyObject*)self;
}

int
cs_RoomObject_init(cs_RoomObject* self, PyObject* args, PyObject* kwds)
{
	self->room = new Room("");
	return 0;
}

PyObject*
PyCSRoom_New(Room* room)
{
	cs_RoomObject* self = (cs_RoomObject*)cs_RoomObject_new(&cs_RoomType,(PyObject*)NULL,(PyObject*)NULL);
	self->room = room;
	return (PyObject*)self;
}

PyObject*
cs_RoomObject_add(cs_RoomObject* self, PyObject* arg)
{
	cs_SocketObject* client = (cs_SocketObject*)arg;
	self->room->add(client->client);
	return (PyObject*)self;
}

PyObject*
cs_RoomObject_remove(cs_RoomObject* self, PyObject* arg)
{
	cs_SocketObject* client = (cs_SocketObject*)arg;
	self->room->remove(client->client);
	return (PyObject*)self;
}

PyObject*
cs_RoomObject_broadcast(cs_RoomObject* self, PyObject* arg)
{
	Message* m = PyCSMessage_AsMessage(arg);
	self->room->broadcast(*m);
	delete m;
	return (PyObject*)self;
}

PyObject*
cs_RoomObject_isMember(cs_RoomObject* self, PyObject* arg)
{
	cs_SocketObject* client = (cs_SocketObject*)arg;
	if (self->room->isMember(client->client)) {
		return Py_True;
	} else {
		return Py_False;
	}
}

PyObject*
cs_RoomObject_isEmpty(cs_RoomObject* self)
{
	if (self->room->isEmpty()) {
		return Py_True;
	} else {
		return Py_False;
	}
}

// Prototypes

extern PyObject* PyCSRoom_New(Room* room);

// cs_ServerObject Methods
//

void
cs_ServerObject_dealloc(cs_ServerObject* self)
{
	// Don't Do Squat...
}

PyObject*
cs_ServerObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	cs_ServerObject* self;
	self = (cs_ServerObject*)type->tp_alloc(type,0);
	if (self == NULL) return (PyObject*)NULL;
	return (PyObject*)self;
}

PyObject*
PyCSServer_New()
{
	cs_ServerObject* self = (cs_ServerObject*)cs_ServerObject_new(&cs_ServerType,(PyObject*)NULL,(PyObject*)NULL);
	return (PyObject*)self;
}

PyObject* 
cs_ServerObject_broadcast(cs_ServerObject* self, PyObject* arg)
{
	Message* m = PyCSMessage_AsMessage(arg);
	Server::broadcast(*m);	
	delete m;
	return (PyObject*)self;
}

PyObject*
cs_ServerObject_im(cs_ServerObject* self, PyObject* arg)
{
	Message* m = PyCSMessage_AsMessage(arg);
	Server::imessage(*m);	
	delete m;
	return (PyObject*)self;
}

PyObject*
cs_ServerObject_addRoom(cs_ServerObject* self, PyObject* arg)
{
	const char* room = PyString_AsString(arg);
	Server::addRoom(room);
	return (PyObject*)self;
}

PyObject*
cs_ServerObject_removeRoom(cs_ServerObject* self, PyObject* arg)
{
	const char* room = PyString_AsString(arg);
	Server::removeRoom(room);
	return (PyObject*)self;
}

PyObject*
cs_ServerObject_room(cs_ServerObject* self, PyObject* arg)
{
	const char* room = PyString_AsString(arg);
	Room* rm = Server::room(room);
	PyObject* retval = PyCSRoom_New(rm);
	return retval;
}

PyObject*
cs_ServerObject_name(cs_ServerObject* self)
{
	PyObject* retval = PyString_FromString(Server::name().c_str());
	return retval;
}


PyObject* PyCSMessage_New(Message& m)
{
	PyObject* dict,*value;
	vector<string>* keys = m.keys();
	vector<string>::iterator i;
	
	dict = PyDict_New();
	if (dict == NULL) {
		delete keys;
		return NULL;
	}
	
	for (i = keys->begin(); i != keys->end(); ++i) {
		value = PyString_FromString(m[*i].c_str());
		PyDict_SetItemString(dict,i->c_str(),value); // ref stolen
	}
	delete keys;
	return dict;	
}

Message* PyCSMessage_AsMessage(PyObject* o)
{
	Message* m = new Message();
	if (! PyDict_Check(o)) {
		cerr << "PyCSMessage_AsMessage invalid param " << endl;
		return m;
	}
	
	PyObject *key, *value;
	Py_ssize_t pos = 0;
	
	while(PyDict_Next(o,&pos,&key,&value)) {
		m->add(PyString_AsString(key),PyString_AsString(value));
	}
	
	return m;
}


// cs_SocketObject Methods
//

void
cs_SocketObject_dealloc(cs_SocketObject* self)
{
	// Don't Do Squat...
}

PyObject*
cs_SocketObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	cs_SocketObject* self;
	self = (cs_SocketObject*)type->tp_alloc(type,0);
	if (self == NULL) return (PyObject*)NULL;
	self->client = NULL;
	return (PyObject*)self;
}

PyObject*
PyCSSocket_New(Socket* c)
{
	cs_SocketObject* self = (cs_SocketObject*)cs_SocketObject_new(&cs_SocketType,(PyObject*)NULL,(PyObject*)NULL);
	self->client = c;
	return (PyObject*)self;
}

PyObject*
cs_SocketObject_send(cs_SocketObject* self, PyObject* arg)
{
	Message* m = PyCSMessage_AsMessage(arg);
	self->client->send(*m);
	delete m;
	return (PyObject*)self;
}

PyObject*
cs_SocketObject_uid(cs_SocketObject* self)
{
	PyObject* pyuid;
	pyuid = PyString_FromString(self->client->login()->uid.c_str());
	return pyuid;
}

