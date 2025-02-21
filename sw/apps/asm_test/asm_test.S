# Test des instructions ajoutées via un exemple en assembleur
.data
.p2align 1
str1: .asciz "Hello, World !"

.text
.p2align 1
f1:
    lui  t2, %hi(str1)
    addi t2, t2, %lo(str1)
    lw   a0, 0(t2)
    jalr x0, ra

.p2align 1
f2:
    lui  t2, %hi(str1)
    addi t2, t2, %lo(str1)
    lw   a0, 4(t2)
    jalr x0, ra

.p2align 1
setup:

    # Instructions du DIFT
    lui  t2, %hi(str1)
    addi t2, t2, %lo(str1)

    ## Marquage de t5 à 1 (x30)
    p.set   t5

    ## Marquage de 'Hell' à 1
    p.spsw  x0, 0(t2)

    ## Marquage de 'or' à 1
    p.spsh  x0, 8(t2)

    ## Marquage de '!' à 1
    p.spsh  x0, 13(t2)

    # Instructions du H-DIFT

    ## Marquage de t4 à la sécurité 10
    addi    t6, x0, 0b10
    p.hmark t4, t6

    ## Marquage de `Hell` à la sécurité 11
    addi    t6, x0, 0b11
    p.hmem  x0, 0(t2)

    ## Changement de sécurité et saut
    # p.hset  x0, t6
    # jal     ra, f1

    ## Changement de sécurité et saut
    lui     t4, %hi(f2)
    addi    t4, t4, %lo(f2)
    p.hmark t4, t6
    jalr    ra, t6

    # Fin
    addi a0, x0, 0
    jalr x0, ra

.p2align 1
loop:
    # Fin
    addi a0, x0, 0
    jalr x0, ra

.globl setup
.globl loop