#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DIFT 1

void secretFunction()
{
  printf("\nShellcode!\nYou have entered in the secret function!\n");
  for(int i = 0; i < 1000; i++) printf("");

  exit(0);
}

void echo()
{
  int a;
  register int i asm("x8");
 
  a = i;

  #ifdef DIFT
  /*TAG INITIALIZATION*/
  asm volatile ("p.spsw x0, 0(%[a]);"                
               :
               :[a] "r" (&a));
  #endif

  printf("%224u%n%35u%n%253u%n%n", 1, (int*) (a-4), 1, (int*) (a-3), 1, (int*) (a-2), (int*) (a-1));

  return;
}

int main(int argc, char* argv[])
{ 
  volatile int a = 1;

  if(a)
    echo();
  else
    secretFunction();

  return 0;
}
