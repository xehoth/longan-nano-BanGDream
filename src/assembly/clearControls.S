.globl clearControls
.type clearControls, @function
clearControls:
	# clear control buffer states
	# reset isPressBoo0
	la t0, isPressBoot0
	sw x0, 0(t0)
	# reset isPressButtons[0]
	la t0, isPressButtons
	sw x0, 0(t0)
	# reset isPressButtons[1]
	sw x0, 4(t0)
	# return
	ret