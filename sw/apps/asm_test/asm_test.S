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
    # Stocke ra
    addi sp, sp, -4
    sw   ra, 0(sp)

    # Appel de printf    
    la      a0, str1
    jal     printf

    # Reprend ra
    lw   ra, 0(sp)
    addi sp, sp, 4
    
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
    p.spsb  x0, 13(t2)

    # Instructions du H-DIFT

    ## Marquage de t4 à la sécurité 10
    p.hmark t4, 0b10

    ## Marquage de `Hell` à la sécurité 11
    p.hmem  x0, 0(t2)

    ## Changement de sécurité et saut
    p.hset  x0, 0b11

    addi sp, sp, -4
    sw   ra, 0(sp)

    jal     ra, f1

    lw   ra, 0(sp)
    addi sp, sp, 4

    ## Changement de sécurité et saut
    
    ### On stocke ra dans le stack
    addi sp, sp, -4
    sw   ra, 0(sp)

    ### On saute !
    addi    t6, x0, 0b11
    lui     t4, %hi(f2)
    addi    t4, t4, %lo(f2)
    p.hmark t4, 0b11         # <- On marque le niveau de sécurité appelé à 3
    jalr    ra, t4

    ### On enlève ra du stack
    lw   ra, 0(sp)
    addi sp, sp, 4

    # Fin
    addi a0, x0, 0
    jalr x0, ra

.p2align 1
loop:

    # Temporisation pour l'UART
    addi    t1, zero, 101
_loop_wait_end:
    addi    t1, t1, -1
    bne     t1, zero, 0  

    # Fin
    addi a0, x0, 0
    jalr x0, ra

.globl setup
.globl loop