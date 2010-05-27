import math
import collections

class Punch:
	"""Represents a punch"""
	def __init__(self,hand,height):
		"""Creates a punch based on the hand"""
		self.hand = hand
		self.height = height
	def __str__(self):
		return ' '.join([self.hand, self.__class__.__name__ , 'to', self.height])
	def ranges(self,distance):
		"""Delivers the range categories based on distance"""
		return {
			5: 'hug',
			10: 'close',
			15: 'near',
			20: 'far',
			25: 'out',
		}[min(25,5*math.ceil(distance/5.0))]
	def powers(self,range):
		"""Stub for the individualized punch powers"""
		return 0
	def power(self,distance,block):
		"""Delivers the actual power rating of the punch"""
		return block * self.powers(self.ranges(distance))
	def zone(self):
		return ({ 
			'left' : 'right',
			'right' : 'left',
		}[self.hand],self.height)
			

class Jab(Punch):
	"""Represents a basic jab"""
	def powers(self,range):
		"""Returns the powers for each range"""
		return {	
			'hug': 1,
			'close': 5,
			'near': 2,
			'far': 0,
			'out': 0,
		}[range]

class Hook(Punch):
	"""Represents all manners of hooks"""
	def powers(self,range):
		"""Returns the powers for each range"""
		return {
			'hug': 0,
			'close': 1,
			'near': 5,
			'far': 4,
			'out': 0,
		}[range]

class UpperCut(Punch):
	"""Represents an uppercut"""
	def powers(self,range):
		"""Returns the powers for each range"""
		return {
			'hug': 2,
			'close': 4,
			'near': 1,
			'far': 0,
			'out': 0,
		}[range]
	def zone(self):
		return ('center',self.height)

class Cross(Punch):
	"""Represents a straight cross punch"""
	def powers(self,range):
		"""Returns the powers for each range"""
		return {
			'hug': 0,
			'close': 1,
			'near': 4,
			'far': 5,
			'out': 0,
		}[range]

class Combo(collections.deque):
	"""Defines a sequence of punches"""
	def __init__(self,punches):
		for p in punches:
			self.append(p)	

c = Combo([
	Jab('left','body'),
	Hook('right','body'),
	UpperCut('left','belt'),
	Cross('left','head') ])

for p in c:
	for r in [3,8,13,18,23]:
		print p,'at',r,'->',p.power(r,1)

