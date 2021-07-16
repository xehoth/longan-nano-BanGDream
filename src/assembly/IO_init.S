.globl IO_init
.type IO_init, @function
IO_init:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra

	call Inp_init 
	# Inp_init();
	call Adc_init 
	# Adc_init();
	call Lcd_Init 
	# Lcd_Init();

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return
