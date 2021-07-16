.globl showNoSdCard
.type showNoSdCard, @function
showNoSdCard:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra

	li a0, 24 
	# first string
	li a1, 0
	la a3, NO_CARD_FOUND
	li a4, 0x0000 
	# black
	call LCD_ShowString 
	# LCD_ShowString(24, 0, (u8 *)("no card found!"), BLACK);

	li a0, 24 
	# second string
	li a1, 16
	la a3, NO_CARD_FOUND
	li a4, 0x001F 
	# blue
	call LCD_ShowString 
	# LCD_ShowString(24, 16, (u8 *)("no card found!"), BLUE);

	li a0, 24 
	# third string
	li a1, 32
	la a3, NO_CARD_FOUND
	li a4, 0xF81F 
	# bred
	call LCD_ShowString 
	# LCD_ShowString(24, 32, (u8 *)("no card found!"), BRED);

	li a0, 24 
	# forth string
	li a1, 48
	la a3, NO_CARD_FOUND
	li a4, 0x07FF 
	# gblue
	call LCD_ShowString 
	# LCD_ShowString(24, 48, (u8 *)("no card found!"), GBLUE);

	li a0, 24 
	# fifth string
	li a1, 64
	la a3, NO_CARD_FOUND
	li a4, 0xF800 
	# red
	call LCD_ShowString 
	# LCD_ShowString(24, 64, (u8 *)("no card found!"), RED);

	call showNoSdCardC

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return

.globl globalInit
.type globalInit, @function
globalInit:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra
	call IO_init 
	# IO_init();
	call init_uart0 
	# init_uart0();

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return
