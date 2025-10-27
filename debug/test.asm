main:   push bp
        mov bp, sp

        mov edx, ds
        mov eax, 1
        mov cx, 1
        ldh ecx, 4

        mov efx, 1
        mov [edx], efx
        sys 2

        mov efx, 0
        mov fl, 1
        mov [edx], efx
        sys 2

        mov efx, -1
        mov [edx], efx
        sys 2

        mov efx, 0
        mov fh, 1
        mov [edx], efx
        sys 2

        mov efx, -1
        mov [edx], 0
        mov b[edx], efx
        sys 2

        mov efx, -1
        mov [edx], 0
        mov w[edx], efx
        sys 2
        
        mov sp, bp
        pop bp
        ret


