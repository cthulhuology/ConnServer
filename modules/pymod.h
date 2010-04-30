// pymod.h
//
// Copyright (C) 2004, 2005 David J. Goehrig
// All rights reserved
//
// python based modules for chat server
//

#ifndef __HAVE_PYMOD__H__
#define __HAVE_PYMOD__H__

#include <Python.h>

#include "module.h"
#include "room.h"

class PyModule : public Module {
	public:
		PyModule(const string& s);
		~PyModule();
		bool process(Socket* c, Message& m);
		static void register_pymodules(Registry* r);
		string name();
	private:
		PyObject *module;
		PyObject *callable;
		string message;
};
// module methods

static PyMethodDef cs_methods[] = {
	{NULL}
};


// Message

PyObject* PyCSMessage_New(Message& m);
Message* PyCSMessage_AsMessage(PyObject* o);

//  Room

typedef struct {
	PyObject_HEAD
	Room* room;
} cs_RoomObject;

PyObject* PyCSRoom_New(Room* room);

extern "C" {
	void cs_RoomObject_dealloc(cs_RoomObject*);
	PyObject* cs_RoomObject_new(PyTypeObject*,PyObject*,PyObject*);
	int cs_RoomObject_init(cs_RoomObject*,PyObject*,PyObject*);
	PyObject* cs_RoomObject_add(cs_RoomObject*,PyObject*);
	PyObject* cs_RoomObject_remove(cs_RoomObject*,PyObject*);
	PyObject* cs_RoomObject_broadcast(cs_RoomObject*,PyObject*);
	PyObject* cs_RoomObject_isMember(cs_RoomObject*,PyObject*);
	PyObject* cs_RoomObject_isEmpty(cs_RoomObject*);
	PyObject* cs_RoomObject_finalize(cs_RoomObject*);
};

// cs_RoomObject Python Object

PyMethodDef cs_RoomObject_methods[] = {
	{ "add", (PyCFunction)cs_RoomObject_add, METH_O, "adds a client to a room" },
	{ "remove", (PyCFunction)cs_RoomObject_remove, METH_O, "removes a client from a room" },
	{ "broadcast", (PyCFunction)cs_RoomObject_broadcast, METH_O, "sends a message to all members in the room" },
	{ "isMember", (PyCFunction)cs_RoomObject_isMember, METH_O, "returns true if the user is a member of the room" },
	{ "isEmpty", (PyCFunction)cs_RoomObject_isEmpty, METH_NOARGS, "returns true if the room is empty" },
	{ "finalize", (PyCFunction)cs_RoomObject_finalize, METH_NOARGS, "deallocates the Room structure if created by __init" },
	{ NULL }
};

PyTypeObject cs_RoomType = {
	PyObject_HEAD_INIT(NULL)
	0,					// ob_size
	"cs.Room",				// tp_name
	sizeof(cs_RoomObject),			// tp_basicsize
	0,					// tp_itemsize
	(destructor)cs_RoomObject_dealloc,	// tp_dealloc
	0,					// tp_print
	0,					// tp_getattr
	0,					// tp_setattr
	0,					// tp_compare
	0,					// tp_repr
	0,					// tp_as_number
	0,					// tp_as_sequence
	0,					// tp_as_mapping
	0,					// tp_hash
	0,					// tp_call
	0,					// tp_str*
	0,					// tp_getattro
	0,					// tp_setattro
	0,					// tp_as_buffer
	Py_TPFLAGS_DEFAULT,			// tp_flags
	"CS Room objects",			// tp_doc
	0,					// tp_traverse
	0,					// tp_clear
	0,					// tp_richcompare
	0,					// tp_weaklistofset
	0,					// tp_iter
	0,					// tp_iternext
	cs_RoomObject_methods,			// tp_methods
	0,					// tp_members	
	0,					// tp_getset
	0,					// tp_base
	0,					// tp_dict
	0,					// tp_descr_get
	0,					// tp_descr_set
	0,					// tp_dictoffset
	(initproc)cs_RoomObject_init,		// tp_init
	0,					// tp_alloc
	cs_RoomObject_new,			// tp_new
};


// PyServer

typedef struct {
	PyObject_HEAD
} cs_ServerObject;


PyObject* PyCSServer_New();

extern "C" {
	void cs_ServerObject_dealloc(cs_ServerObject*);
	PyObject* cs_ServerObject_new(PyTypeObject*,PyObject*,PyObject*);
	PyObject* cs_ServerObject_broadcast(cs_ServerObject*,PyObject*);
	PyObject* cs_ServerObject_im(cs_ServerObject*,PyObject*);
	PyObject* cs_ServerObject_addRoom(cs_ServerObject*,PyObject*);
	PyObject* cs_ServerObject_removeRoom(cs_ServerObject*,PyObject*);
	PyObject* cs_ServerObject_room(cs_ServerObject*,PyObject*);
	PyObject* cs_ServerObject_name(cs_ServerObject*);
};

// Methods and Members

PyMethodDef cs_ServerObject_methods[] = {
	{ "broadcast", (PyCFunction)cs_ServerObject_broadcast, METH_O,"broadcasts a message to all clients" },
	{ "im", (PyCFunction)cs_ServerObject_im, METH_O, "sends a message to the given room or client" },
	{ "addRoom", (PyCFunction)cs_ServerObject_addRoom, METH_O, "adds the given room to the server" },
	{ "removeRoom", (PyCFunction)cs_ServerObject_removeRoom, METH_O, "removes the given room from the server"},
	{ "room", (PyCFunction)cs_ServerObject_room, METH_O, "returns the room object associated with the given name"},
	{ "name", (PyCFunction)cs_ServerObject_name, METH_NOARGS, "returns the name of the server"},
	{ NULL }
};


// cs_ServerObject Python Object

PyTypeObject cs_ServerType = {
	PyObject_HEAD_INIT(NULL)
	0,					// ob_size
	"cs.Server",				// tp_name
	sizeof(cs_ServerObject),		// tp_basicsize
	0,					// tp_itemsize
	(destructor)cs_ServerObject_dealloc,	// tp_dealloc
	0,					// tp_print
	0,					// tp_getattr
	0,					// tp_setattr
	0,					// tp_compare
	0,					// tp_repr
	0,					// tp_as_number
	0,					// tp_as_sequence
	0,					// tp_as_mapping
	0,					// tp_hash
	0,					// tp_call
	0,					// tp_str*
	0,					// tp_getattro
	0,					// tp_setattro
	0,					// tp_as_buffer
	Py_TPFLAGS_DEFAULT,			// tp_flags
	"CS Server objects",			// tp_doc
	0,					// tp_traverse
	0,					// tp_clear
	0,					// tp_richcompare
	0,					// tp_weaklistofset
	0,					// tp_iter
	0,					// tp_iternext
	cs_ServerObject_methods,		// tp_methods
	0,					// tp_members	
	0,					// tp_getset
	0,					// tp_base
	0,					// tp_dict
	0,					// tp_descr_get
	0,					// tp_descr_set
	0,					// tp_dictoffset
	0,					// tp_init
	0,					// tp_alloc
	cs_ServerObject_new,			// tp_new
};


// Socket 

typedef struct {
	PyObject_HEAD
	Socket* client;
} cs_SocketObject;

PyObject* PyCSSocket_New(Socket* c);

extern "C" {
	void cs_SocketObject_dealloc(cs_SocketObject*);
	PyObject* cs_SocketObject_new(PyTypeObject*,PyObject*,PyObject*);
	PyObject* cs_SocketObject_send(cs_SocketObject*,PyObject*);
	PyObject* cs_SocketObject_uid(cs_SocketObject*);
};

// cs_SocketObject Python Object

PyMethodDef cs_SocketObject_methods[] = {
	{ "send", (PyCFunction)cs_SocketObject_send, METH_O, "sends a message to the client" },
	{ "uid", (PyCFunction)cs_SocketObject_uid, METH_NOARGS, "returns the uid of the associated client" },
	{ NULL }
};

PyTypeObject cs_SocketType = {
	PyObject_HEAD_INIT(NULL)
	0,					// ob_size
	"cs.Socket",				// tp_name
	sizeof(cs_SocketObject),		// tp_basicsize
	0,					// tp_itemsize
	(destructor)cs_SocketObject_dealloc,	// tp_dealloc
	0,					// tp_print
	0,					// tp_getattr
	0,					// tp_setattr
	0,					// tp_compare
	0,					// tp_repr
	0,					// tp_as_number
	0,					// tp_as_sequence
	0,					// tp_as_mapping
	0,					// tp_hash
	0,					// tp_call
	0,					// tp_str*
	0,					// tp_getattro
	0,					// tp_setattro
	0,					// tp_as_buffer
	Py_TPFLAGS_DEFAULT,			// tp_flags
	"CS Socket objects",			// tp_doc
	0,					// tp_traverse
	0,					// tp_clear
	0,					// tp_richcompare
	0,					// tp_weaklistofset
	0,					// tp_iter
	0,					// tp_iternext
	cs_SocketObject_methods,		// tp_methods
	0,					// tp_members	
	0,					// tp_getset
	0,					// tp_base
	0,					// tp_dict
	0,					// tp_descr_get
	0,					// tp_descr_set
	0,					// tp_dictoffset
	0,					// tp_init
	0,					// tp_alloc
	cs_SocketObject_new,			// tp_new
};

#endif
