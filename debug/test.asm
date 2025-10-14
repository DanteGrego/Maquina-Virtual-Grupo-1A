main:       sys 0xf
            mov edx, DS
            mov eax, 0x01
            ldh ecx, 4
            ldl ecx, 2
            sys 1
            push [edx]
            push [edx+4]
            ;retorna eax = (sp+4)**(sp)
            call powIter
            add sp, 4
            add sp, 4
            mov [DS+8], eax
            push [edx]
            push [edx+4]
            ;retorna eax = (sp+4)**(sp)
            call powRecu
            add sp, 4
            add sp, 4
            mov [DS+12], eax
            mov eax, 0x01
            mov edx, DS
            add edx, 8
            ldh ecx, 4
            ldl ecx, 2
            sys 2;muestra primero el resultado iterativo y luego el recursivo
            ret

powIter:    push BP
            mov BP, SP
            sub SP, 4
            push ebx
            push ecx
            mov [BP-4], 1
            mov ecx, [BP+8];ecx = b
            mov ebx, [BP+12];ebx = a
sigue:      cmp ecx, 0
            jnp finPow
            mul [BP-4], ebx
            sub ecx, 1
            jmp sigue
finPow:     mov eax, [BP-4]
            pop ecx
            pop ebx
            add SP, 4
            mov SP, BP
            pop BP
            ret     


powRecu:    push BP
            mov BP, SP
            sub SP, 4
            mov [BP-4], 1;res
            cmp [BP+8], 0;b>0
            jnp finRecu
            sub [BP+8], 1
            push [BP-4]
            push [BP+8]
            call powRecu
            add SP, 4
            add SP, 4
            mul [BP-4], [BP+12];res*=a
            mul [BP-4], eax;en eax estaba a**(b-1)
finRecu:    mov eax, [BP-4]
            add SP, 4
            mov SP, BP
            pop BP
            ret     