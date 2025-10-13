main:           mov eax, 10
                mov ecx, 10
                mov edx, ds
                add eax, ecx
                mov [ds], eax
                mov eax, 1
                ldl ecx, 1
                ldh ecx, 4
                sys 2
                stop

imprimo:        