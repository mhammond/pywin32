# Start of a basic time module.

# For now we support enough for pystone!

__first_time = COR.System.DateTime.Now
__ticks_per_ms = COR.System.Convert.ToDouble(COR.System.TimeSpan.TicksPerMillisecond)

def clock(): 
#	d = COR.System.DateTime.op_Subtraction(COR.System.DateTime.Now, __first_time)
	n = COR.System.DateTime.Now
	d = n.Subtract(__first_time)
	ms = COR.System.Convert.ToInt32(d.Ticks) / __ticks_per_ms
	return ms / 1000.0
