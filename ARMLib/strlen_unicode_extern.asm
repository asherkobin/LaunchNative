	AREA	|.text|, CODE, READONLY
	EXPORT	arm_strlen_unicode

ReverseBytePos \
        dcw      7, 6, 5, 4, 3, 2, 1, 0


arm_strlen_unicode
		ldrh		w2,[x0],#0
		cbz			w2,null_string
		mov         x5,x0  
		tbnz        w0,#0,branch0 
		ands        x1,x5,#0xF  
		beq         branch1  
		and         x2,x5,#0xFFF  
		cmp         x2,#0xFF0  
		bgt         branch2
		ld1         {v0.8h},[x5]  
		uminv       h1,v0.8h  
		fmov        w2,s1
		cbz         w2,branch3
		add         x5,x5,#0x10  
		and         x5,x5,#-0x10  
		nop
branch1
		ld1         {v0.8h},[x5],#0x10  
		uminv       h1,v0.8h  
		fmov        w2,s1  
		cbnz        w2,branch1
		sub         x5,x5,#0x10  
branch3
		ldr         q1,ReverseBytePos
		cmeq        v0.8h,v0.8h,#0  
		and         v0.16b,v0.16b,v1.16b  
		umaxv       h0,v0.8h  
		fmov        w2,s0  
		eor         w2,w2,#7  
		sub         x0,x5,x0  
		add         x0,x2,x0,asr #1  
		ret
branch2
		sub         x1,x1,#0x10  
		neg         x1,x1,asr #1  
branch6
		ldrh        w2,[x5],#2  
		cbz         w2,branch5
		subs        x1,x1,#1  
		bgt         branch6
		b           branch1
branch0
		ldrh        w2,[x5],#2  
		cbnz        w2,branch0
branch5
		sub         x5,x5,#2  
		sub         x0,x5,x0  
		asr         x0,x0,#1  
		ret

ret
		mov			x5,x0

		cbz         x0,null_string			; do nothing for a NULL sring

		ldrsh		x8,[x0]					; load first character into x8
		mov			x9,x0					; save x0 into x9
		cbz			x8,zero_length_string	; handle if string is "\0"

non_zero_character
		ldrsh		x8,[x9,#2]!				; load next character into x8
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