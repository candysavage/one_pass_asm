.global _start
.extern labela1,labela2,exit,arraybase

.equ simbolLiteral,0x1234+labela2-labela1+op1

.text
_start:
    jmp *main(%pc)
    mov $0x0, %r0
    call exit

main: push %r5
    mov %sp, %r5
    mov 0x8(%r5), %r2
    mov (%r2), storage
    mov op1, %r0
    mov $1, %r1
    xchg %r0, %r1
    add %r1, %r0
    call function
    pop %r5
    ret

function:
	mov op1, %r1
	shl $2, %r2
loop:
	add arraybase(%r2), %r1
	sub $1, %r2
	test %r2, %r2
	jne loop
	mov %r1, %r0
	ret

.bss
.skip 0x20

.data
op1:.byte 0xff,1
labela5:
    .word labela5,0x1234
    .word 0xffff,-512
storage:
    .word 0

.end

