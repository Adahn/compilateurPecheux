	inc	0
	jp	main
f2:
	libp	-4
	libp	0
	pushr	1.000000
	move	1
	libp	1
	pushr	2.000000
	move	1
	libp	-3
	copy	1
	pushr	100.000000
	add
	move	1
	ret
main:
	inc	1
	pushr	22.000000
	pushr	4.000000
	savebp
	inc	2
	call	f2
	dec	2
	rstrbp
	dec	2
	output
	halt
	end
