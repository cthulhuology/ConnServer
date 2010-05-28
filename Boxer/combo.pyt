# combo.pyt
# coding=utf8
#
# © 2010 David J. Goehrig
#

from combo import *
import unittest

class ComboTests(unittest.TestCase):
	def runTest(self):
		c = Combo([
			Jab('left','body'),
			Hook('right','body'),
			UpperCut('left','belt'),
			Cross('left','head') ])
		assert len(c) == 4
		assert c.cost() == 12
		c.clear()
		assert len(c.random()) > 0
		assert c.random().cost() > 0

if __name__ == "__main__":
	unittest.main()
