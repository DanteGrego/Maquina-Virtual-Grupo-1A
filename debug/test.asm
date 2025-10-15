TEXTO1 equ "Hola mundo aaaaaa\n"

        mov bp, sp
        push edx
        mov edx, KS
        add edx, TEXTO1
        sys 0x4
        pop edx
        mov sp, bp
        pop bp
        ret
main:   push bp
        mov bp, sp
        push edx
        mov edx, KS
        add edx, TEXTO1
        sys 0x4
        pop edx
        sys 0xf
        mov sp, bp
        pop bp
        ret