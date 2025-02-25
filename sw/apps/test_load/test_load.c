// Test d'un load pour voir si la mémoire protégée est atteinte
#include "stdlib.h"
int a = 0;
int b = 2;

int main() {
    int* a1 = &a;
    int* b1 = &b;
    
    *b1 = *a1;
    *a1 = *b1;
}