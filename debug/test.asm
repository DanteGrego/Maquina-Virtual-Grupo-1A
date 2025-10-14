main:           sys 0xf
                mov eax, 10
                add eax, 15
                mov edx, ds
                mov [edx], eax
                call imprimo
                stop

imprimo:        push bp
                mov bp, sp
                push eax
                push ecx
                mov eax, 1
                ldh ecx, 4
                ldl ecx, 1
                sys 2
                pop ecx
                pop eax
                pop bp
                ret