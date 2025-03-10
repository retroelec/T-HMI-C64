* = $033f

	cpx	#1
	beq	error
	ora	#0
	beq	filenotfound
	lda	#141
	jsr	$e716
	lda	#<loadedtext
	ldy	#>loadedtext
printtext
	jsr	$ab1e
	lda	#141
	jmp	$e716
filenotfound
	lda	#141
	jsr	$e716
	lda	#<filenotfoundtext
	ldy	#>filenotfoundtext
	jmp	printtext
error
	lda	#141
	jsr	$e716
	lda	#<errortext
	ldy	#>errortext
	jmp	printtext

loadedtext
	.text	"loaded"
	.byte	0
filenotfoundtext
	.text	"file not found"
	.byte	0
errortext
	.text	"error"
	.byte	0

