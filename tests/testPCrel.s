.global labela5,labela4
.extern labela6

.text
.skip 0x10
labela1:
	jmp *labela2(%pc)
labela4:
labela2:
	call *labela1(%pc)
	addw labela5(%r7), labela3(%r7)
	subb %r0h, labela6
	jgt *labela4(%pc)

.data
.skip 16
labela3:	.word 0x1234
labela5:
.end
