        mov eax, 2
        ldh ecx, 1
        ldl ecx, 4
        mov edx, ds
        sys 1
        mov eax, 31
        sys 2
        stop