    AREA    |.text|, CODE, READONLY
    EXPORT  check_zero

check_zero
    CBZ     w0, is_zero     ; If input param == 0, jump

not_zero:
    MOV     w0, #1
    RET

is_zero:
    MOV     w0, #42
    RET