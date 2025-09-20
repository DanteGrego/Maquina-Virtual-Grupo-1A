        mov edx, ds
        ldh ecx, 2
        ldl ecx, 1
        mov eax, 0x1F
        mov [edx], 0xaabbccdd
        sys 2
        stop