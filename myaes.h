// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Wraps openssl AES functions to provide simple char * interface.

#ifndef MYAES_H
#define MYAES_H

#include <iostream>
#include <cstring>
#include <openssl/evp.h>

template<typename T>
int aes_encrypt(T   key           , T cleartext , \
                int cleartext_len , T ciphertext) {

	unsigned char iv[32] = { 0 };
	int len, flen;
	EVP_CIPHER_CTX en;
	EVP_CIPHER_CTX_init(&en);
	EVP_EncryptInit_ex(&en, EVP_aes_256_cbc(), NULL, key, iv);
	EVP_EncryptInit_ex(&en, NULL, NULL, NULL, NULL);
	EVP_EncryptUpdate(&en, ciphertext, &len          , \
	                       cleartext , cleartext_len ) ;
  EVP_EncryptFinal_ex(&en, ciphertext+len, &flen);
	EVP_CIPHER_CTX_cleanup(&en);
	return len + flen;
}

template<typename T>
int aes_decrypt(T key              , T ciphertext, \
                int ciphertext_len , T cleartext ) {
  unsigned char iv[32] = { 0 };
  int len, flen;
  EVP_CIPHER_CTX de;
  EVP_CIPHER_CTX_init(&de);
  EVP_DecryptInit_ex(&de, EVP_aes_256_cbc(), NULL, key, iv);
  EVP_DecryptInit_ex(&de, NULL, NULL, NULL, NULL);
  EVP_DecryptUpdate(&de, cleartext , &len           , \
                         ciphertext, ciphertext_len ) ;
	EVP_DecryptFinal_ex(&de, cleartext+len, &flen);
	EVP_CIPHER_CTX_cleanup(&de);
	return len + flen;
}

#endif
