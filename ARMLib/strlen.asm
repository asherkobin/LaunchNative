	AREA	|.text|, CODE, READONLY
	EXPORT	strlen

strlen

	cbz         x0,null_string			; do nothing for a NULL sring

	ldrsb		x8,[x1]					; load first character into x8
	mov			x9,x1					; save x1 into x9
	cbz			x8,zero_length_string	; handle if string is "\0"
non_zero_character
	ldrsb		x8,[x9,#1]!				; load next character into x8
	cbnz		x8,non_zero_character	; compare and branch if x8 is > 0x00
	sub			x8,x9,x1				; this will give the count of characters in x8
	mov			x0,x8					; x1 is returned to the calller

zero_length_string
null_string
	ret									; mission complee!

END