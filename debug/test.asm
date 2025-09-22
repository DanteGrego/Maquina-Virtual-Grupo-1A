        mov ebx, CS
        add ebx, 11
        mov efx, 10
        mov eax, 1
        mov edx, DS
        mov [edx], efx
        ldl ecx, 1
        ldh ecx, 4
        sys 2
        sub efx, 1
        cmp efx, 0
        jz fin
        mov ip, ebx

fin:    stop