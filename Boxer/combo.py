# combo.py
# coding=utf8
#
# © 2010 David J. Goehrig
#

import collections
import random
from punch import *

class Combo(collections.deque):
	"""Defines a sequence of punches"""
	def random(self):
		for i in range(random.sample(xrange(5),1)[0]+1):	
			self.append(random.sample([
				Jab(self.arm(),self.target()),
				Hook(self.arm(),self.target()),
				UpperCut(self.arm(),self.target()),
				Cross(self.arm(),self.target())	
			],1)[0])
		return self
	def arm(self):
		"""Returns a random arm"""
		return random.sample(['left','right'],1)[0]
	def target(self):
		"""Returns a random height"""
		return random.sample(['head','body','belt'],1)[0]
	def dump(self):
		"""Prints out a dump of the contents of a combo"""
		for p in self:
			print p
		return self
	def cost(self):
		return sum(map(lambda x: x.cost(), self))
