* = $033f

	ora	#0
	beq	filenotfound
	lda	#141
	jsr	$e716
	lda	#<loadedtext
	ldy	#>loadedtext
	jsr	$ab1e
	lda	#141
	jmp	$e716
filenotfound
	lda	#141
	jsr	$e716
	lda	#<filenotfoundtext
	ldy	#>filenotfoundtext
	jsr	$ab1e
	lda	#141
	jmp	$e716

loadedtext
	.text	"loaded"
	.byte	0
filenotfoundtext
	.text	"file not found"
	.byte	0

