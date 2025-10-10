inicio: sys 0xF
        mov EDX, DS
        ldh ECX, 0x04
        ldl ECX, 0x01
        mov EAX, 0x01
        sys 0x1
        mov EFX, [DS]
        mov EDX, DS
        add EDX, 4
        ldh ECX, 0x04
        ldl ECX, EFX
        mov EAX, 0x01
        sys 0x1
        mov EDX, DS
        add EDX, 4
        ldh ECX, 0x04
        ldl ECX, EFX
        mov EAX, 0x01
        sys 0x2
        stop