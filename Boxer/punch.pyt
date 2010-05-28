# punch.pyt
# coding=utf8
#
# © 2010 David J. Goehrig
#

from punch import *

class JabTest(unittest.TestCase):
	def runTest(self):
		c = Jab('left','head')
		assert c.powers('hug') == 1
		assert c.powers('close') == 5
		assert c.powers('near') == 2
		assert c.powers('far') == 0

class HookTest(unittest.TestCase):
	def runTest(self):
		c = Hook('left','head')
		assert c.powers('hug') == 0
		assert c.powers('close') == 1
		assert c.powers('near') == 5
		assert c.powers('far') == 4

class UpperCutTest(unittest.TestCase):
	def runTest(self):
		c = UpperCut('left','head')
		assert c.powers('hug') == 2
		assert c.powers('close') == 4
		assert c.powers('near') == 1
		assert c.powers('far') == 0
		assert c.zone() == ('center','head')

class CrossTest(unittest.TestCase):
	def runTest(self):
		c = Cross('left','head')
		assert c.powers('hug') == 0
		assert c.powers('close') == 1
		assert c.powers('near') == 4
		assert c.powers('far') == 5

if __name__ == "__main__":
	unittest.main()
