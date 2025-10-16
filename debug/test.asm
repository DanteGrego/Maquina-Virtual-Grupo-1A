        SYS 0xF       
        mov b[0], 0
        mov b[1], -1
        mov b[2], 2
        mov b[3], -3
        mov b[4], 4
        mov b[5], -5
        mov b[6], 6
        mov b[7], -7
        mov b[8], 8
        mov b[9], -9
        mov b[10], 10
        mov b[11], -11

        mov eax, b[4]
        mov ebx, b[6]
        mov ecx, w[9]
        mov edx, w[0]
        sys 0xF
        mov fl, b[2]
        mov fh, b[3]
        ldh efx, w[5]
        stop
