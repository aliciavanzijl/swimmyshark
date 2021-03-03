  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.
  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw		\reg,0($sp)
.endm

.macro	POP reg
	lw		\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #
  .data
  .text
  
# ENABLE INTERRUPT FUNCTION
# by Alicia van Zijl 2021/02/22
.global enable_interrupt
enable_interrupt:
	ei
	jr $ra
	nop
	
  
# HEXASC FUNCTION
# by Alicia van Zijl
.global hexasc
hexasc:
	andi  $t1, $a0, 0x0f	# makes sure t1 has only the 4 LSB of a0 containing a value
	addi  $v0, $zero, 0x30	# makes sure v0 contains the hex for ASCII 0 by default - 0x30
	addi  $t0, $zero, 0x9	# sets t0 to 9

     ble   $t1, $t0,	hexnum	# branch if a0 less or equal to t0(9) - jumps to returning a number
     nop
     addi  $v0, $v0, 0x7	# adds an extra 7 to 0x30, giving 0x37 = 55, the amount to add to make letters

hexnum:
     add   $v0, $t1, $v0	# adds v0 (the number or letter base) and a0 (the original value) and puts in v0
     jr    $ra		# return to the original function
	 nop			# delay slot filler
	
# DELAY FUNCTION (assignment 4)
# by Alicia van Zijl
.global delay
delay:
	PUSH $ra
	PUSH $a0
	
	# while loop
while_begin:
	slt 	$t0, $0, $a0		# while ms > 0, $t0 = 1
	beq	$t0, $0, while_end		# if ms !> 0, end the while loop
	nop
	# for loop
	addi 	$a0, $a0, -1		# ms = ms - 1
	add	$t1, $t1, $0			# i = 0
	addi	$t2, $0, 10000	# SET CONSTANT
for_begin:
	slt	$t3, $t1, $t2			# if i(t1) < constant(t2), t3 = 1
	beq	$t3, $0, for_end 		# if i !< constant, end the for loop
	# do nothing for 1 milisecond
	nop 
	addi	$t1, $t1, 1			# i = i + 1
	
	j	for_begin
	nop
	
for_end:
	j	while_begin
	nop
	
while_end:
	POP $a0
	POP $ra
	
	jr	$ra
	nop

