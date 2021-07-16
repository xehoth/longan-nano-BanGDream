.globl init_uart0
.type init_uart0, @function
init_uart0:
	# init uart0 for serial printf
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra
	# enable GPIO clock
	li a0, 1538 
	# RCU_GPIOA = 1538
	call rcu_periph_clock_enable 
	# rcu_periph_clock_enable(RCU_GPIOA);
	li a0, 1550 
	# RCU_USART0 = 1550
	call rcu_periph_clock_enable
	# connect port to USARTx_Tx
	li a0, 1073809408 
	# GPIOA
	li a1, 0x18
	# GPIO_MODE_AF_PP
	li a2, 0x03 
	# GPIO_OSPEED_50MHZ
	li a3, 512 
	# GPIO_PIN_9
	call gpio_init 
	# gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

	# connect port to USARTx_Rx
	li a0, 1073809408 
	# GPIOA
	li a1, 0x04 
	# GPIO_MODE_IN_FLOATING
	li a2, 0x03 
	# GPIO_OSPEED_50MHZ
	li a3, 1024 
	# GPIO_PIN_10
	call gpio_init 
	# gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);

	# USART configure
	li a0, 1073821696 
	# USART0
	call usart_deinit 
	# usart_deinit(USART0);

	li a0, 1073821696 
	# USART0
	li a1, 128000 
	# monitor speed
	call usart_baudrate_set 
	# usart_baudrate_set(USART0, 128000U);

	li a0, 1073821696 
	# USART0
	li a1, 0 
	# USART_WL_8BIT
	call usart_word_length_set 
	# usart_word_length_set(USART0, USART_WL_8BIT);

	li a0, 1073821696 
	# USART0
	li a1, 0 
	# USART_STB_1BIT
	call usart_stop_bit_set 
	# usart_stop_bit_set(USART0, USART_STB_1BIT);

	li a0, 1073821696 
	# USART0
	li a1, 0 
	# USART_PM_NONE
	call usart_parity_config 
	# usart_parity_config(USART0, USART_PM_NONE);

	li a0, 1073821696 
	# USART0
	li a1, 0 
	# USART_RTS_DISABLE
	call usart_hardware_flow_rts_config 
	# usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);

	li a0, 1073821696 
	# USART0
	li a1, 0 
	# USART_CTS_DISABLE
	call usart_hardware_flow_cts_config 
	# usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);

	li a0, 1073821696 
	# USART0
	li a1, 4 
	# USART_RECEIVE_ENABLE
	call usart_receive_config 
	# usart_receive_config(USART0, USART_RECEIVE_ENABLE);

	li a0, 1073821696 
	# USART0
	li a1, 8 
	# USART_TRANSMIT_ENABLE
	call usart_transmit_config 
	# usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

	li a0, 1073821696 
	# USART0
	call usart_enable 
	# usart_enable(USART0);

	li a0, 1073821696 
	# USART0
	li a1, 773 
	# USART_INT_RBNE
	call usart_interrupt_enable 
	# usart_interrupt_enable(USART0, USART_INT_RBNE);

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret

.globl onInitRiscvBefore
.type onInitRiscvBefore, @function
onInitRiscvBefore:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra

	la t0, currentFrame
	sw x0, 0(t0) 
	# currentFrame = 0

	la t0, currentScene
	sw x0, 0(t0)
	# currentScene = MENU_SCENE

	call clearControls
	# clearControls()

	li a0, 0x0000 
	# BLACK
	call LCD_Clear 
	# LCD_Clear(BLACK);

	call showLonganLogo 
	# showLonganLogo();

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret

.globl onInitRiscvAfter
.type onInitRiscvAfter, @function
onInitRiscvAfter:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra

	li a0, 1500
	call delay_1ms 
	# delay_1ms(1500);

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return

.globl mainGameStartRiscv
.type mainGameStartRiscv, @function
mainGameStartRiscv:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra

	la t0, perfectCount
	sw x0, 0(t0) 
	# perfectCount = 0;
  la t0, greatCount
	sw x0, 0(t0) 
	# greatCount = 0;
	la t0, goodCount
  sw x0, 0(t0) 
	# goodCount = 0;
  la t0, missCount
	sw x0, 0(t0) 
	# missCount = 0;
  la t0, combo 
	sw x0, 0(t0) 
	# combo = 0;
	la t0, score
  sw x0, 0(t0) 
	# score = 0;
  li t1, 180
	la t0, changeToScoreScene 
	sw t1, 0(t0) 
	# changeToScoreScene = 180;
	la t0, noteBufferBegin
	sw x0, 0(t0)
  sw x0, 4(t0) 
	# noteBufferBegin[0] = noteBufferBegin[1] = 0;
	la t0, checkCoolDown
	sw x0, 0(t0)
	sw x0, 4(t0) 
	# checkCoolDown[0] = checkCoolDown[1] = 0;
	li a0, 0
	call loadNote 
	# loadNote(0);
  li a0, 1 
	call loadNote 
	# loadNote(1);

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return

.globl onReleaseRiscv
.type onReleaseRiscv, @function
onReleaseRiscv:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra

	la a0, bgTextureFile
	call f_close 
	# f_close(&bgTextureFile);
	la a0, numberTextureFile
	call f_close
	# f_close(&numberTextureFile);

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return

.globl gameMenuRiscv
.type gameMenuRiscv, @function
gameMenuRiscv:
	addi sp, sp, -4
	sw ra, 0(sp) 
	# save ra

	# delay_1ms(1000);
	li a0, 1000
	call delay_1ms

	gameMenuRiscv_loop:
	# for (;;) {
	call updateControls
	# updateControls();
	call checkAutoPlayMode
	# checkAutoPlayMode();
	# 	isPressBoot0
	la t0, isPressBoot0
	lw t0, 0(t0)
	beq x0, t0, gameMenuRiscv_loop_else

	# do is (isPressBoot0)
	la t0, currentScene
	# MAIN_GAME_SCENE
	li t1, 1
	sw t1, 0(t0)
	# currentScene = MAIN_GAME_SCENE;
	la t0, currentFrame
	sw x0, 0(t0)
	# currentFrame = 0;
	call mainGameStart
	# mainGameStart();
	j gameMenuRiscv_loop_end
	# break;

	gameMenuRiscv_loop_else:
	call changeMusic
	# changeMusic();
	call clearControls
	# clearControls();
	j gameMenuRiscv_loop

	gameMenuRiscv_loop_end:

	lw ra, 0(sp) 
	# reload ra
	addi sp, sp, 4
	ret 
	# return

.globl mainFuncRiscv
.type mainFuncRiscv, @function
mainFuncRiscv:
	addi sp, sp, -4
	sw ra, 0(sp)
	# save ra

	call globalInit
	# globalInit();
	mainFuncRiscv_outside_loop:

	call mountSdCard
	# mountSdCard();

	la t0, sdCardFr
	lw t0, 0(t0)

	bne x0, t0, mainFuncRiscv_outside_loop_else
	# sdCardFr != 0
		call onInit
		# onInit()

		mainFuncRiscv_inside_loop:
			call gameMenu 
			# gameMenu();
			call mainGameLoop
			# mainGameLoop();
			call scoreScene
			# scoreScene();

		j mainFuncRiscv_inside_loop

	mainFuncRiscv_outside_loop_else:
		call showNoSdCard
		# showNoSdCard();

	j mainFuncRiscv_outside_loop
	mainFuncRiscv_outside_loop_end:

	lw ra, 0(sp)
	# reload ra
	addi sp, sp, 4
	ret
	# return

.globl drawScore
.type drawScore, @function
drawScore:
	addi sp, sp, -12
	sw ra, 0(sp)
	# save ra
	sw s0, 4(sp)
	# save s0
	sw s1, 8(sp)
	# save s1

	mv s0, a0
	# s0 = side

	la s1, score
	lw s1, 0(s1)
	# s1 = score

	li t1, 1
	bne s0, t1, drawScore_side_neq_1
		# side == 1
		mv a0, s0
		# side
		li t0, 10000
		rem t0, s1, t0
		# t0 = score % 10000
		li t1, 1000
		div t0, t0, t1
		# t0 = left1 = (score % 10000) / 1000
		addi a1, t0, 300
		# NUMBER_TEXTURE = 300
		li a2, 27
		li a3, 140
		call renderTexture
		# renderTexture(side, NUMBER_TEXTURE + left1, 27, 140);
		mv a0, s0
		# side
		li t0, 10000
		div t0, s1, t0
		# t0 = left0 = score / 10000
		addi a1, t0, 300
		# NUMBER_TEXTURE = 300
		li a2, 14
		li a3, 140
		call renderTexture
		# renderTexture(side, NUMBER_TEXTURE + left0, 14, 140);

	drawScore_side_neq_1:

	# mid
	mv a0, s0
	# side
	li t0, 1000
	rem t0, s1, t0
	# t0 = score % 1000
	li t1, 100
	div t0, t0, t1
	# t0 = mid = (score % 1000) / 100
	addi a1, t0, 300
	# NUMBER_TEXTURE = 300
	li a2, 40
	li a3, 140
	call renderTexture
	# renderTexture(side, NUMBER_TEXTURE + mid, 40, 140);

	bne x0, s0, drawScore_side_neq_0
	# side == 0
		mv a0, s0
		# side
		li t0, 100
		rem t0, s1, t0
		# t0 = score % 100
		li t1, 10
		div t0, t0, t1
		# t0 = right0 = (score % 100) / 10
		addi a1, t0, 300
		# NUMBER_TEXTURE = 300
		li a2, 53
		li a3, 140
		call renderTexture
		# renderTexture(side, NUMBER_TEXTURE + right0, 53, 140);

		mv a0, s0
		# side
		li t0, 10
		rem t0, s1, t0
		# t0 = right1 = score % 10
		addi a1, t0, 300
		# NUMBER_TEXTURE = 300
		li a2, 66
		li a3, 140
		call renderTexture
		# renderTexture(side, NUMBER_TEXTURE + right1, 66, 140);

	drawScore_side_neq_0:

	lw s1, 8(sp)
	# reload s1
	lw s0, 4(sp)
	# reload s0
	lw ra, 0(sp)
	# reload ra
	addi sp, sp, 12
	ret
	# return