// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Create a general purpose cryptosystem by combining ecdh and aes

// FIXME: Need to derive the key from the shared secret using standard methods

#ifndef ECDH_AES_H
#define ECDH_AES_H

#include "curve25519.h"
#include "myaes.h"

int encrypt_once(unsigned char * server_public, \
  unsigned char * cleartext, int cleartext_len, unsigned char * packet) {

	unsigned char client_secret[32];
	unsigned char client_public[32];
	unsigned char shared[32];

	curve25519_keypairgen(client_secret, client_public);
	curve25519(shared, client_secret, server_public);

	unsigned char ciphertext[1024];

	int ci_len = aes_encrypt(shared, 32, cleartext, cleartext_len, ciphertext);

	memcpy(packet, client_public, 32);
	memcpy(packet + 32, ciphertext, ci_len);
	return ci_len + 32;
}

int decrypt_once(unsigned char * server_private, unsigned char * packet, \
 int packet_len, unsigned char * decrypted) {

	unsigned char shared[32];
	curve25519(shared, server_private, packet);
	return aes_decrypt(shared, 32, packet + 32, packet_len - 32, decrypted);
}

class cryptopeer {
	public:
		cryptopeer() :    my_temp_pvt_ecdh_key(0) , \
		                  my_temp_pub_ecdh_key(0) , \
		               their_temp_pub_ecdh_key(0) , \
		               their_perm_pub_ecdh_key(0) , \
		                           our_aes_key(0) {

		}

	private:

		char    my_temp_pvt_ecdh_key[32];
		char    my_temp_pub_ecdh_key[32];
		char their_temp_pub_ecdh_key[32];
		char their_perm_pub_ecdh_key[32];
		char             our_aes_key;

};

#endif
