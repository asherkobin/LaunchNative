	AREA	|.text|, CODE, READONLY
	EXPORT	arm_strlen

arm_strlen
		cbz         x0,null_string			; do nothing for a NULL sring

		ldrsb		x8,[x0]					; load first character into x8
		mov			x9,x0					; save x0 into x9
		cbz			x8,zero_length_string	; handle if string is "\0"

non_zero_character
		ldrsb		x8,[x9,#2]!				; load next character into x8
		cbnz		x8,non_zero_character	; compare and branch if x8 is > 0x00
		sub			x8,x9,x0				; this will give the count of characters in x8
		mov			x0,x8					; x0 is returned to the calller
		ret

zero_length_string
		mov			x0,#0
		ret

null_string
		mov			x0,#0
		ret									; mission complee!

END