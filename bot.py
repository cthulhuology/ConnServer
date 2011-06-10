#!/usr/bin/env python
#
#	bot.py
#
# 	Copyright (C) 2005 David J. Goehrig
# 	All Rights Reserved
#
# 	Poker bots for fun and profit
#

from posix import getpid
from fcntl import *
from time import sleep,time
from signal import SIGIO,SIGALRM,signal,alarm
from messages import *
from sys import exit

def handler(signum,frame):
	Bot.delay_flag = True

def iohandler(signum,frame):
	Bot.io_flag = True

class Bot:
	delay = 1	# delay between heartbeats
	delay_flag = False
	io_flag = False
	def __init__(self,a,p,g):
		print '[BOT INIT ]',a,p,g
		self.statemachine = g
		self.done = False
		self.conn = Connection(a,p)
		self.login()
		self.statemachine.set_connection(self.conn)
		self.cps = self.statemachine.setup()
		self.heartbeats = 0
		self.lifespan = 10
	def login(self):
		print '[LOGIN]'
		m = Message()
		m.login(self.statemachine.game.table.id)
		self.conn.send(m)
		m = self.conn.wait_message('bot-login')
		if not m or m['status'] != '0':
			print "Failed to login"
			exit(1)	
	def logout(self):
		print '[LOGOUT]'
		exit(0)
	def start(self):
		fcntl(self.conn.sock.fileno(),F_SETOWN,getpid())
		flgs = fcntl(self.conn.sock.fileno(),F_GETFL,0)
		fcntl(self.conn.sock.fileno(),F_SETFL,flgs | 0x0040 | 0x0004) # O_ASYNC | O_NONBLOCK
		signal(SIGALRM,handler)
		signal(SIGIO,iohandler)
		alarm(Bot.delay)
		self.listen()
	def heartbeat(self):
		print '[Heartbeat]', Bot.delay_flag
		if not Bot.delay_flag:
			return
		Bot.delay_flag = False
		self.cps = self.statemachine.heartbeat(self.heartbeats,self.cps)
		self.heartbeats += 1
		alarm(Bot.delay)

	def listen(self):
		while True:
			try:
				if Bot.io_flag or Bot.delay_flag:
					if Bot.delay_flag:
						self.heartbeat()
						Bot.delay_flag = False
					m = False
					m = self.conn.read_message()
					if self.conn.done:
						raise "Lost connection"
					if m:
						self.cps = self.statemachine.process(self.cps,m)
						if m['msg'] != 'pong':
							self.heartbeats = 0
					else:
						Bot.io_flag = False
				else:
					t = time()
					print "[listen] SLEEP"
					sleep(10)
					print "[listen] Elapsed: ", int(time()-t)
			except RuntimeError:
				self.cps = self.statemachine.setup()

### END ###
