            mov ebx, 1
            mov eax, 0x01
            mov edx, DS
            ldl ecx, 0x01
            ldh ecx, 0x04
            sys 1

            mov efx, 2
            mov eex, [0]
otro:       mov eax, eex
            div eax, efx
            cmp ac, 0
            jz noespri
            cmp eax, 1
            jnz espri
            add efx, 1
            jmp otro
noespri:    mov ebx, 0
espri:      mov [0], ebx
            mov eax, 0x01
            mov edx, DS
            ldl ecx, 0x01
            ldh ecx, 0x04
            sys 2
            stop
