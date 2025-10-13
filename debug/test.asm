main:   sys 0xf
        mov eax, 10
        mov ecx, 10
        add eax, ecx
        mov [ds], eax
        mov edx, ds
        mov eax, 1
        ldl ecx, 1
        ldh ecx, 4
        sys 2
        stop