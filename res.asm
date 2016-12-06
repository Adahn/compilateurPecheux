	inc	1
	jp	main
fibonacci: 
	libp	0
	mts
	pushr	3.000000
	ls
	jf	L000
	ret
	jp	L001
L000:
	ret
L001:
main:
	push	0
	pushr	1.000000
	stm
L002:
	push	0
	mts
	pushr	10.000000
	ls
	jf	L003
	inc 1
	savebp
	inc 0
	call fibonacci
	dec 0 
	rstrbp
	dec 1 
	output
	push	0
	push	0
	mts
	pushr	1.000000
	add
	stm
	jp	L002
L003:
	halt
	end
