# ai.py
# coding=utf8
#
# © 2010 David J. Goehrig
#

from boxer import *

class AI(Boxer):
	def __init__(self,msg):
		super(AI,self).__init__(msg)
		self.seen = {
			'jab': 0,
			'hook': 0,
			'uppercut': 0,
			'cross': 0
		}
		self.distance = 0
		self.damage = {
			'done' : 0,
			'taken' : 0,
			'blocked' : 0,
		}
		self.punches = {
			'thrown' : {
				'left' : 0,
				'right' : 0,
				'head' : 0,
				'body' : 0,
				'belt' : 0,
			}
			'connected' : {
				'left' : 0,
				'right' : 0,
				'head' : 0,
				'body' : 0,
				'belt' : 0,
			},
			'blocked' : {
				'left' : 0,
				'right' : 0,
				'head' : 0,
				'body' : 0,
				'belt' : 0,
			},
		}
		self.limits = {
			'feet': 50,
			'fists': 50,
			'grit': 50,
			'guts': 50,
			'smarts': 50,
		}
	def attacks(self):
		self.selection = Combo().random()
		self.selection.dump()
	def defenses(self):
		self.arms = { 'left' : 'down', 'right' : 'down' }
	def movements(self):
		self.move(1)
		self.position[x] += 1
	def think(self):
		
