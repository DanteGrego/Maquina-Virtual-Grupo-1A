textoargv equ "argv:\n"
textoargc equ "argc:\n"
textosep equ "\n"

main:   push bp
        mov bp, sp
        push eax
        push ecx
        push edx
        push efx
        mov edx, KS
        add edx, textoargc
        sys 4
        mov edx, BP
        add edx, 8
        mov eax, 0x08
        ldh ecx, 0x4
        mov cx, 1
        sys 2
        mov efx, edx
        mov edx, KS
        add edx, textoargv
        sys 4
        mov edx, efx
        add edx, 4
        sys 2
        mov ecx, [BP+8]
        cmp ecx, 0
        jz fin
        mov efx, [edx]
sigue:  mov edx, [efx]
        sys 4
        mov edx, KS
        add edx, textosep
        sys 4
        sub ecx, 1
        add efx, 4
        cmp ecx, 0
        jnz sigue
fin:    pop efx
        pop edx
        pop ecx
        pop eax
        mov sp, bp
        pop bp
        ret

        
