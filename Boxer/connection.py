#!/usr/bin/env python
# coding=utf8
#
# © 2010 David J. Goehrig
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
		
