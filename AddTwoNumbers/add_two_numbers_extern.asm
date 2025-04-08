	AREA	|.text|, CODE, READONLY
	EXPORT	add_two_numbers

add_two_numbers
    
	mov x0, x0
	mov x1, x1
	add x0, x0, x1

	ret

END