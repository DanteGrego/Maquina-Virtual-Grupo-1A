t1 equ "hola como estas\n"
t2 equ "dale un 10 porfa\n"
t3 equ "nos dolio el 99.35 :(\n"
t4 equ "hola mundo\n"
t5 equ "bom dia mundo\n"
t6 equ "konnichiwa sekai\n"
t7 equ "hello world\n"

main:   mov edx, KS
        add edx, t1
        sys 4

        mov edx, KS
        add edx, t2
        sys 4

        mov edx, KS
        add edx, t3
        sys 4

        mov edx, KS
        add edx, t4
        sys 4
        mov edx, KS
        add edx, t5
        sys 4
        mov edx, KS
        add edx, t6
        sys 4
        mov edx, KS
        add edx, t7
        sys 4

        mov edx, DS
        rnd [edx], 10
        mov eax, 0x01
        ldh ecx, 4
        ldl ecx, 1
        sys 2
        sys 0xf
        mov [DS], [SP+8]
        sys 2

        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        pop [DS]
        sys 2
        mov sp, bp 
        pop bp
        ret