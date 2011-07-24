/*
test-curve25519 version 20050915
D. J. Bernstein
Public domain.
*/

#include <stdio.h>
#include "curve25519.h"

void doit(unsigned char *ek,unsigned char *e,unsigned char *k)
{
  int i;
  for (i = 0;i < 32;++i) printf("%02x",(unsigned int) e[i]); printf(" ");
  for (i = 0;i < 32;++i) printf("%02x",(unsigned int) k[i]); printf(" ");
  curve25519(ek,e,k);
  for (i = 0;i < 32;++i) printf("%02x",(unsigned int) ek[i]); printf("\n");
}

unsigned char e1k[32];
unsigned char e2k[32];
unsigned char e1e2k[32];
unsigned char e2e1k[32];
unsigned char e1[32] = {3};
unsigned char e2[32] = {5};
unsigned char k[32] = {9};

main()
{
  int loop;
  int i;

  for (loop = 0;loop < 1000000000;++loop) {
    doit(e1k,e1,k);
    doit(e2e1k,e2,e1k);
    doit(e2k,e2,k);
    doit(e1e2k,e1,e2k);
    for (i = 0;i < 32;++i) if (e1e2k[i] != e2e1k[i]) printf("fail\n");
    for (i = 0;i < 32;++i) e1[i] ^= e2k[i];
    for (i = 0;i < 32;++i) e2[i] ^= e1k[i];
    for (i = 0;i < 32;++i) k[i] ^= e1e2k[i];
  }

  return 0;
}
