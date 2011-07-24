// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// wrap openssl base64 functions for easy char * to char * usage.

#ifndef BASE64_H
#define BASE64_H

#include <iostream>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

template<typename T> int base64_encode(T len, char * message, char * encoded) {

	BUF_MEM *bptr = NULL;

	BIO * b64  = BIO_new(BIO_f_base64());
   BIO * bmem = BIO_new(BIO_s_mem());
	b64        = BIO_push(b64, bmem);
	BIO_write(b64, message, len);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	memcpy(encoded, bptr->data, bptr->length == 0 ? 0 : bptr->length -1);
	encoded[bptr->length - 1] = '\n';

	int res = bptr->length;

	BIO_free_all(b64);
	return res;
}

template<typename T> int base64_decode(T len, char * encoded, char * decoded) {

	BIO * b642 = BIO_new(BIO_f_base64());
	BIO *bmem2 = BIO_new_mem_buf(encoded, len);
	bmem2      = BIO_push(b642, bmem2);
	int inlen = BIO_read(bmem2, decoded, len);

	BIO_free_all(bmem2);
	return inlen;
}

#endif
