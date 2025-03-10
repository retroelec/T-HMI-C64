* = $033f

	ora	#0
	beq	error
	lda	#141
	jsr	$e716
	lda	#<savedtext
	ldy	#>savedtext
printtext
	jsr	$ab1e
	lda	#141
	jmp	$e716
error
	lda	#141
	jsr	$e716
	lda	#<errortext
	ldy	#>errortext
	jmp	printtext

savedtext
	.text	"saved"
	.byte	0
errortext
	.text	"error"
	.byte	0

