// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.
// The curve25519 library that is program uses was originally written by
// Daniel J. Bernstein. No license information was provided in the package.

// Provides wrapper around the curve25519 library for simple char * usage.

#ifndef CURVE25519_H
#define CURVE25519_H

#include <iostream>
#include <cstdlib>
#include <cstring>

// fix a very annoying slight mismatch between prototype
// and implementation that gcc doesn't pick up on
extern "C" void curve25519_athlon(unsigned char*, unsigned char const*, unsigned char const*);

#include "curve25519/curve25519.h"

template<typename T>
void curve25519_keypairgen(T secret, \
                           T pub   ) {

	memset(pub, 0, 32);

	for (size_t i = 0; i < 32; i++) {
		secret[i] = rand() % 256;
	}
	secret[0 ] &= 248;
	secret[31] &= 127;
	secret[31] |= 64;

	const unsigned char basepoint[32] = {9};
	curve25519(pub, secret, basepoint);
}

template<typename T>
void curve25519_keypairgen(T keypair) {
	curve25519_keypairgen(keypair, keypair + 32);
}

#endif
