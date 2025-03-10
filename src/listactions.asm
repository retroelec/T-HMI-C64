* = $033f

	jmp	main
filename
	.repeat 17,0
main
	cpx	#1
	beq	printstart
	cpx	#2
	beq	printnext
	lda	#<filename
	ldy	#>filename
printtext
	jsr	$ab1e
	lda	#141
	jmp	$e716
printstart
	lda	#141
	jsr	$e716
	lda	#<starttext
	ldy	#>starttext
	jmp	printtext
printnext
	lda	#141
	jsr	$e716
	lda	#<nexttext
	ldy	#>nexttext
	jmp	printtext

starttext
	.text	"*** start"
	.byte	0
nexttext
	.text	"*** next"
	.byte	0

