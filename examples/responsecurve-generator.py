#!/usr/bin/env python

import sys
import string

if len(sys.argv) != 3:
    print "Usage:", sys.argv[0], "STEPS", "EQUATION"
    print "Simple generator for generating responsecurve data from equations."
    print ""
    print "Example:"
    print "   ", sys.argv[0], "6 i**2"
else:
    steps = int(sys.argv[1])
    equation = sys.argv[2]

    left  = [int(eval(equation, {'i': i/float(steps-1)}) * -32768) for i in range(0,steps)]
    right = [int(eval(equation, {'i': i/float(steps-1)}) *  32767) for i in range(0,steps)]

    left.reverse()
    left = left[0:-1]
    
    print string.join([str(x) for x in (left + right)], ":")
    
# EOF #
