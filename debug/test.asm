main:   mov eax, 10
        mov ecx, 10
        add eax, ecx
        mov W[ds], eax
        mov eax, 1
        ret
        push al
        pop ax
        call main
        ldl ecx, 1
        ldh ecx, 4
        sys 2
        stop