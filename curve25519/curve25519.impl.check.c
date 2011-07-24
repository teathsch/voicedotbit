/*
curve25519.impl.check.c version 20050915
D. J. Bernstein
Public domain.
*/

#include "curve25519.impl.check.h"

unsigned char e1k[32];
unsigned char e2k[32];
unsigned char e1e2k[32];
unsigned char e2e1k[32];
unsigned char e1[32] = {3};
unsigned char e2[32] = {5};
unsigned char k[32] = {9};

unsigned char exp[32] = {
  0xbe,0x4c,0x62,0x08,0x29,0x3f,0x81,0x1a,
  0x15,0x4b,0x9c,0x42,0xf7,0x87,0xdd,0x90,
  0x9f,0x07,0x5c,0x61,0x1b,0x82,0xc3,0x03,
  0x50,0xed,0xc9,0xfe,0x6e,0x83,0xad,0x4a,
};

main()
{
  int loop;
  int i;

  for (loop = 0;loop < 10;++loop) {
    curve25519(e1k,e1,k);
    curve25519(e2e1k,e2,e1k);
    curve25519(e2k,e2,k);
    curve25519(e1e2k,e1,e2k);
    for (i = 0;i < 32;++i) if (e1e2k[i] != e2e1k[i]) return 1;
    for (i = 0;i < 32;++i) e1[i] ^= e2k[i];
    for (i = 0;i < 32;++i) e2[i] ^= e1k[i];
    for (i = 0;i < 32;++i) k[i] ^= e1e2k[i];
  }
  for (i = 0;i < 32;++i) if (e1e2k[i] != exp[i]) return 1;

  return 0;
}
