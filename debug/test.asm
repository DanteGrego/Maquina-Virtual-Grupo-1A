\\DATA 4
\\EXTRA 0
\\STACK 0


    mov edx, DS
    sys 0xf
    mov [edx], 0
    add [edx], 'h'
    shl [edx], 8
    add [edx], 'o'
    shl [edx], 8
    add [edx], 'l'
    shl [edx], 8
    add [edx], 'a'
    mov eax, 2
    ldh ecx, 4
    ldl ecx, 1
    sys 2
    stop


