#!/usr/bin/env python
#
#	Copyright (C) 2005 David J. Goehrig
#	All rights reserved
#
#	Message Passing routines
#

from sys import exit
from socket import *

class Connection:
	"Manages the connection to the chatroom"
	def __init__(self,address,port):
		self.sock = socket()
		self.sock.connect((address,port))
		self.sock.setblocking(0)
		self.done = False
	def send(self,m):
		m.send(self.sock)
	def recv(self):
		msg = ''
		while msg[-1:] != "~" and msg[-1:] != "\0":
			try:
				chunk = self.sock.recv(1)
				if chunk == '':
					print "Connection lost"
					exit(1)
			except IOError:
				raise IOError
			except error:
				return ''
			msg += chunk
		return msg[:-1]
	def read_message(self):
		t = ''
		m = False
		try:
			t = self.recv()
		except IOError:
			self.done = True
		if len(t) > 0:
			m = Message()
			m.from_string(t)
		return m
	def wait_message(self,msg):
		print '[Wait Message',msg,']'
		m = self.read_message()
		while not self.done and (not m or m['msg'] != msg):
			m = self.read_message()
		return m
		

class Message:
	def __init__(self,d={}):
		elmts = {}
		rep = ""
		self.from_dict(d)
	def __str__(self):
		return self.rep
	def __len__(self):
		return self.elmts.__len__()
	def __getitem__(self,key):
		return self.elmts[key]
	def __setitem__(self,key,value):
		return self.elmts.__setitem__(key,value)
	def __delitem__(self,key):
		return self.elmts.__delitem__(key)
	def __iter__(self):
		return self.elmts.__iter__()
	def __contains__(self,item):
		return self.elmts.__contains__(item)
	def from_dict(self,d):
		self.elmts = dict(d)
		v = list()
		for k in self.elmts:
			v.append(str(k) + ":" + str(self.elmts[k]))
		self.rep = ",".join(v)  + "~" 
	def from_string(self,s):
		while s[-1] == '~':
			s = s.rstrip('~')
		while s[0] == '\x00':
			s = s[1:]
		# print "from string >>>>> ", s
		self.rep = s
		self.elmts = dict()
		l = s.split(",")
		for e in l:
			print "spliting ", e
			(k,v) = e.split(":")
			self.elmts[k] = v
	def to_dict(self):
		return self.elmts
	def send(self,sock):
		sock.sendall(self.rep + '\0')	
	def status(self,s):
		self.elmts['status'] = s
		self.from_dict(self.elmts)	
	def ping(self):
		self.from_dict({ 'msg' : 'ping' })
	def login(self,r):
		self.from_dict({ 'msg' : 'bot-login', 'room' : r })
	def logout(self):
		self.from_dict({ 'msg' : 'bot-logout' })
