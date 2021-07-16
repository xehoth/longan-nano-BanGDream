.globl Inp_init
.type Inp_init, @function
Inp_init:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra
	li a0, 1073809408 
	# GPIOA
	li a1, 0x04 
	# GPIO_MODE_IN_FLOATING
	li a2, 0x03 
	# GPIO_OSPEED_50MHZ
	li a3, 256 
	# GPIO_PIN_8
	call gpio_init 
	# gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return
