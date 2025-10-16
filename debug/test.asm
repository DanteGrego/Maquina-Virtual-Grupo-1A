main:   sys 0xf
        push bp
        mov bp, sp
        push eax
        push ecx
        push edx
        mov edx, BP
        add edx, 8
        mov eax, 0x08
        ldh ecx, 0x4
        mov cx, 1
        sys 2
        add edx, 4
        sys 2
        mov ecx, [BP+8]
        cmp ecx, 0
        jz fin
        sys 0xf
        mov edx, [edx]
        mov edx, [edx]
        sys 4
sigue:  sys 4
        sub ecx, 1
        add edx, 4
        cmp ecx, 0
        jnz sigue
fin:    pop edx
        pop ecx
        pop eax
        mov sp, bp
        pop bp
        ret

        
