# -*- Mode: Python; tab-width: 4 -*-
#
# Note that this demo will NOT run from Python.exe - this is simply
# because the demo sets up a timer, then exits - if run from Python, the
# process terminates before the first timer event fires.
# AFAIK, there are _no_ Pythonwin requirements for the timer - just this demo!
#
# To run this demo, start Pythonwin, open this file, and select "File/Run",
# and press "Enter".  The interactive window will respond with:
# >>> import timer_demo
# x = 0
# x = 1
# x = 2
# (etc...)

# This module, and the timer.pyd core timer support, were written by
# Sam Rushing (rushing@nightmare.com)

import timer

# glork holds a simple counter for us.

class glork:
	
	def __init__ (self, delay=1000, max=10):
		self.x = 0
		self.max = max
		self.id = timer.set_timer (delay, self.increment)

	def increment (self, id, time):
		print 'x = %d' % self.x
		self.x = self.x + 1
		# if we've reached the max count,
		# kill off the timer.
		if self.x > self.max:
			# we could have used 'self.id' here, too
			timer.kill_timer (id)

# create a counter that will count from '1' thru '10', incrementing
# once a second, and then stop.

def demo (delay=1000, stop=10):
	g = glork(delay, stop)

if __name__=='__main__':
	demo()
