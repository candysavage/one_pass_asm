.global a,c
.extern e,f
.equ dvanaest,12
.equ expr,-s+mam-0x1234+10

.data
a:
    .skip 0x10
b:    .word a,0x4321
    .byte 0xff,-1
c:

#komentar

.text
start:
    push %r5
    mov %sp, %r5
    test %r0, %r0
d:  add dvanaest(%r5), %r0
    sub %r0, %r0

    call *start(%pc)     #PC relativan skok
    int d
    jmp *d
g:  
    shlb $1, %r0l
    shrb %r0h, $1
s:
mam:
.end