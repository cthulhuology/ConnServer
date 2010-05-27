#!/usr/bin/python

from time import clock
from messages import Message
from math import sqrt

class Boxer:
	"""Represents a boxer in all of the scripts"""
	def __init__(self,msg):
		"""Creates a boxer object from a message"""
		self.id = msg['id']
		self.name = msg['name']
		self.nick = msg['nick']
		self.feet = msg['feet']
		self.fists = msg['fists']
		self.grit = msg['grit']
		self.guts = msg['guts']
		self.smarts = msg['smarts']
		self.arms = { 'left' : 'down', 'right' : 'down' }
		self.stats = { 'feet' : 100.0, 'fists' : 100.0, 'grit' : 100.0, 'guts' : 100.0, 'smarts' : 100.0 }
		self.position = { 'x' : 0, 'y' : 0 }
	def heartbeat(self):
		"""Updates a boxer's stats based on their stats"""
		self.stats = { 
			'feet'   : min(100,self.stats['feet'] + (self.feet+self.stats['guts'])/1000.0),
			'fists'  : min(100,self.stats['fists'] + (self.fists+self.stats['guts'])/1000.0),
			'grit'   : min(100,self.stats['grit'] + (self.grit+self.stats['guts'])/1000.0),
			'guts'   : min(100,self.stats['guts'] + (self.guts+self.stats['guts'])/1000.0),
			'smarts' : min(100,self.stats['smarts'] + (self.smarts+self.stats['guts'])/1000.0),
		}
	def move(self,distance):
		"""Moves the character a distance, range is typically 1-5"""
		delta = min(self.stats['feet'],distance)
		self.stats['feet'] = max(0,self.stats['feet'] - delta)
		return delta
	def punch(self,power):
		"""Delivers a punch to an opponent, assumes already hit, power ranges from 1-5"""
		damage = self.stats['fists'] * power**(1 + self.stats['smarts']/100) / 100
		self.stats['fists'] = max(0,self.stats['fists'] -  power)
		return damage
	def feint(self,power):
		"""Fakes out an opponent, wears out their guts, power ranges from 1-5"""
		damage = self.stats['feet'] * power**(1 + self.stats['smarts']/100) / 100
		self.stats['feet'] = max(0,self.stats['feet'] -  power)
		return damage
	def mock(self,power):
		"""Mock an opponent to make them loose their cool"""
		damage = self.stats['smarts'] * power**(1 + self.stats['smarts']/100) / 100
	def punched(self,damage):
		"""A punched boxer takes physical damage represented by grit"""
		self.stats['grit'] = max(0,self.stats['grit'] - damage)
	def psyched(self,damage):
		"""A psyched boxer takes morale damage represented by guts"""
		self.stats['guts'] = max(0,self.stats['guts'] - damage)
	def taunted(self,damage):
		"""A taunted boxer takes mental damage represented by smarts"""
		self.stats['smarts'] = max(0,self.stats['smarts'] - damage)
	def block(self,punch):
		"""Determines if the punch is blocked or not"""
		if self.arms[punch.hand] == punch.zone():
			return 0
		return 1
	def range(self,boxer): 
		"""Determines the distance between boxers"""
		return sqrt((self.position.x - boxer.position.x)**2 + (self.position.y - boxer.position.y)**2)
	def power(self,punch,boxer):
		"""Determine the power of a punch based on zone"""
		return punch.power(self.range(boxer), self.block(punch))


m = Message()
m.from_dict({'fists': 23, 'smarts': 44, 'name': 'Bobby', 'guts': 52, 'feet': 34, 'nick': 'da Thrilla', 'grit': 66, 'id': 123})

b = Boxer(m)

