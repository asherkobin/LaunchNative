	AREA	|.text|, CODE, READONLY
	EXPORT	add_two_numbers

add_two_numbers
    mov x0, #7
	mov x1, #4
	add x2, x0, x1

	mov x0, x2

	ret

END