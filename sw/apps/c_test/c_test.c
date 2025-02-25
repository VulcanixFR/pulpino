#include <stdio.h>

char str1[32] __attribute__((aligned (4)))
    = "Bonjour, je suis du C.\r\n";

void f1 () {
    const char* addr = str1;
    char c = addr[0];
}

void f2 () {
    const char* addr = str1;
    printf(addr);
}
 
int main () {

    // Test de p.set 
    asm (
        "p.set t5\n\t"
        :
    );
    
    // Test de p.spsw
    asm (
        "p.spsw x0, 0(%[txt])\n\t"
        :
        : [txt] "r" (str1)
    );

    // Test de p.spsh
    asm (
        "p.spsh x0, 8(%[txt])\n\t"
        :
        : [txt] "r" (str1)
    );

    // Test de p.spsb
    asm (
        "p.spsb x0, 13(%[txt])\n\t"
        :
        : [txt] "r" (str1)
    );

    // Test de p.hmem
    asm (
        "p.hmem x0, 0(%[txt])\n\t"
        :
        : [txt] "r" (str1)
    );

    // Test de p.hset
    asm (
        "p.hset x0, 0b10\n\t"
        :
    );
    f1();

    // void* func = &f2;
    // // Test de p.hmark
    // asm (
    //     "addi   sp, sp, -4\n\t"
    //     "sw     ra, 0(sp)\n\t"
    //     "lui    t4, %%hi(%[func])\n\t"
    //     "addi   t4, t4, %%lo(%[func])\n\t"
    //     "p.hmark t4, 0b10\n\t"
    //     "jalr   ra, t4\n\t"
    //     "lw     ra, 0(sp)\n\t"
    //     "addi   sp, sp, 4\n\t"
    //     :
    //     : [func] "r" (func)
    // );

    return 0; 
}