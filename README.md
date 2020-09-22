# One pass assembler for CISC architecture
Little endian
****
usage: asm src.s -o obj.o

compilation: g++ -o bin/asm src/*.cpp
****

****
Supports the following
****

_label1:

.extern a,b,c

.global d,e

.equ symbolLiteral, d-e+0x1234-128



.text [#comment]

.section .bss

[#comment]

[label:].byte 0xff,-1 [#comment]

[label:].word 0xffff,-1234,a,symbolLiteral

*****
Architecture details
*****

8 16bit registers r0-7

pc = r7

sp = r6


Operations with one, two or no operand[s]

Maximum 7 bytes long

| InstrDescr | Op1Descr | Im/Di/Ad | Im/Di/Ad | Op2Descr | Im/Di/Ad | Im/Di/Ad |

instruction coding


OC4 OC3 OC2 OC1 OC0 S X X

OpDescr

AM2 AM1 AM0 R3 R2 R1 R0 L/H

AM = addressMode

0x00 - immed

0x01 - regdir

0x02 - regind

0x03 - regind with 16b mov

0x04 - memdir

R3R2R1R0 - regs r0-r7, psw = 0xF

L/H - with regdir and 1 byte operand, lower or higher byte of register, L = 0 H = 1



Mnemonic    OC

halt        0

iret        1

ret         2

inst dst    3

call dst    4

jmp dst     5

jeq dst     6

jne dst     7

jgt dst     8

push src    9

pop src     10

xchg src,dst  11

mov src,dst 12

add src,dst 13

sub src.dst 14

mul src,dst 15

div src,dst 16

cmp src,dst 17

not src,dst 18

and src,dst 19

or  src,dst 20

xor src,dst 21

test src,dst 22

shl src,dst 23


shr dst,src 24


All 2 operand instructions can have w or b next to mnemonic to indicate operand size.

The following syntax is used for addressing in :

# Instructions that access data

$[literal] - immed value [literal]

$[symbol] - immed value [symbol]

%r[num] - regdir

(%r[num]) - regind

[literal](%r[num]) - regind with 16b signed mov [literal] + r[num]

[symbol](%r[num]) - regind with 16b signed mov [symbol] + r[num]

[symbol](%r7/%pc) - read from memory at [symbol] (PC relative)

[literal] - memdir absolute

[symbol] - memdir absolute

#Instructions that represent a jump

[literal] - jump to address [literal]

[symbol] - jump to address [symbol]

*%r[num] - jump to address from register r[num]

*(%r[num])- jump to address read from memory value r[num]

*[literal](%r[num]) - jump to memory at address [literal] + r[num]

*[symbol](%r[num]) - jump to memory at address [symbol] + r[num]

*[symbol](%r7/%pc) - jump to memory at address [symbol] (PC Relative jump)

*[literal] - jump to address read from memory at [literal] (absolute)

*[symbol] - jump to address read from memory at [symbol] (absolute)
