main:   push bp
        mov bp, sp
        push ecx
        push edx
        sys 0xf
        sub sp, 8
        mov edx, BP
        sub edx, 8
        mov cx, 8
        sys 0xf
        sys 0xf
        sys 3
        sys 0xf
        sys 4
        pop edx
        pop ecx
        sys 0xf
        mov sp, bp
        pop bp
        ret


