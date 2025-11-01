t1 equ "ciento veintiocho\n"
        PUSH                BP
        MOV                 BP,                 SP
        SUB                 SP,                  4
        PUSH               EDX
        MOV                EDX,            l[BP+8]
        MOV            l[BP-4],           l[BP+12]
        ADD            l[BP-4],                EDX
        MOV                EAX,            l[BP-4]
        POP                EDX
        MOV                 SP,                 BP
        POP                 BP
        ret
        MOV                 BP,                 -1 
        MOV                EDX,                 DS 
        PUSH               485
        PUSH                64
        SYS                 15
        CALL                 0
        ADD                 SP,                  8
        MOV             l[EDX],                EAX
        MOV           l[EDX+4],                 ES
        MOV           l[EDX+8],                 SP
        LDH                ECX,                  4
        LDL                ECX,                  3
        MOV                EAX,                  9
        SYS                  2
        MOV                EDX,                 SP
        SUB                EDX,                 24
        LDL                ECX,                  6
        SYS                  2