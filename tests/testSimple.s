.global labela3
.extern labela5,labela6
.equ test1, -labela1+0x4311-1+labela4+text

.text
.skip 0x10
labela1: addw test1, labela3(%r7)

.data
labela4:
.word 0x1234,128
.byte -1
labela2: .word labela2
labela3:
.end
