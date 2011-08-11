// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Wrap elliptic curve crypto functions

#ifndef ECDHCRYPTO_H
#define ECDHCRYPTO_H

#include <iostream>
#include <cstring>
#include <stdint.h>
#include <stdexcept>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include "scopeguard.h"

template<typename ExceptionType> class ec_exception_t : public ExceptionType {
	public:
		ec_exception_t(const char * m) : ExceptionType(m) {}
}; typedef ec_exception_t<std::runtime_error> ec_exception;

template<typename SizeType>
void * derive_key_from_shared_secret( const void * in , SizeType   inlen  , \
                                      void       * out, SizeType * outlen ) {

	// Generated from random.org
	uint16_t salt[4] = { 18211 , 10773 , 51513 , 15330 };

	unsigned char iv[32];
	memset(iv, 0, 32); // It gets set, but we don't use it.

	/* if ( */ (*outlen = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), \
	 (unsigned char *) salt, (unsigned char *) in, inlen, 5 , \
	 (unsigned char *) out, iv)); /*  == 0) */
//		throw ec_exception("EVP_BytesToKey returned 0 bytes for key.");
//  Can't throw exception in callback.. It calls terminate().

	return out;
}

template<typename UChar>
int do_generate_ec_keys(UChar * pvt_key, UChar * pub_key) {

	EC_KEY * key = NULL;
	BIGNUM * pub_key_bn = NULL;

	// The crypto free functions check for NULL so we don't have to.
	scope_guard key_guard(std::tr1::bind(&EC_KEY_free, key));
	scope_guard bn_guard(std::tr1::bind(&BN_free, pub_key_bn));

	if ((key = EC_KEY_new_by_curve_name(NID_secp256k1)) == NULL)
		throw ec_exception("EC_KEY_new_by_curve_name failed!");

	if (!EC_KEY_generate_key(key))
		throw ec_exception("EC_KEY_generate_key failed!");

	if ((pub_key_bn = EC_POINT_point2bn(EC_KEY_get0_group(key), \
	                                EC_KEY_get0_public_key(key), \
	                       POINT_CONVERSION_COMPRESSED, NULL, NULL)) == NULL)
		throw ec_exception("EC_POINT_point2bn failed!");

	UChar temp_pub_key[33];

	if (BN_bn2bin(pub_key_bn, temp_pub_key) == 0)
		throw ec_exception("BN_bn2bin returned 0 length for pubkey!");

	memcpy(pub_key, temp_pub_key + 1, 32);

	return BN_bn2bin(EC_KEY_get0_private_key(key), pvt_key);
}

template<typename UChar>
void generate_ec_keys(UChar * pvt_key, UChar * pub_key) {

	for (int len = 0; true; )
		if ((len = do_generate_ec_keys(pvt_key, pub_key)) == 32)
			return;

	throw ec_exception("Got wrong private key size 10 times in a row!");
}

template<typename UChar>
void binary_pvt_key_to_key(const UChar * pvt_key, EC_KEY * key) {

	BIGNUM   * pvt_key_bn = NULL;

	scope_guard pvt_key_guard(std::tr1::bind(BN_free , pvt_key_bn));

	if ((pvt_key_bn = BN_bin2bn(pvt_key, 32, NULL)) == NULL)
		throw ec_exception("BN_bin2bn returned NULL in binary_pvt_key_to_key!");

	// no docs on what this returns
	EC_KEY_set_private_key(key, pvt_key_bn);
}

template<typename UChar> BIGNUM * pub_key_to_bn(const UChar * pub_key,
                    const unsigned char first_byte = 2) {

	BIGNUM * res = NULL;

	UChar temp_pub_key[33];
	temp_pub_key[0] = first_byte;

	memcpy(temp_pub_key + 1, pub_key, 32);

	if ((res = BN_bin2bn(temp_pub_key, 33, NULL)) == NULL)
		throw ec_exception("BN_bin2bn returned NULL in pub_key_to_bn!");

	return res;
}

template<typename UChar> EC_POINT * bn_to_pt(UChar * pub_key, EC_KEY * key,
                                  const unsigned char first_byte = 2) {

	BIGNUM * bn = NULL;
	EC_POINT * res = NULL;

	scope_guard bn_guard(std::tr1::bind(BN_free, bn));
	bn = pub_key_to_bn(pub_key, first_byte);

	if ((res = EC_POINT_bn2point(EC_KEY_get0_group(key), bn, NULL, NULL)) \
	       == NULL)
		throw ec_exception("EC_POINT_bn2point returned NULL in bn_to_pt!");

	return res;
}

template<typename UChar> void compute_ec_shared_secret(const UChar * my_pvt_key,
       const UChar * their_pub_key, UChar * shared_secret) {

	EC_KEY   * my_key       = NULL;
	EC_POINT * their_pub_pt = NULL;

	scope_guard pub_pt_guard(std::tr1::bind( EC_POINT_free , their_pub_pt ));
	scope_guard my_key_guard(std::tr1::bind( EC_KEY_free   , my_key       ));

	if ((my_key = EC_KEY_new_by_curve_name(NID_secp256k1)) == NULL)
		throw ec_exception("EC_KEY_new_by_curve_name failed in compute secret!");

	binary_pvt_key_to_key(my_pvt_key, my_key);

	their_pub_pt = bn_to_pt(their_pub_key, my_key, 2);

	if (ECDH_compute_key(shared_secret, 32, their_pub_pt,
	                         my_key, derive_key_from_shared_secret<size_t>) == 0)
		throw ec_exception("ECDH_compute_key failed!");
}

template<typename UChar> int ec_sign(const UChar * pvt_key, const UChar * msg,
                                     const int     len    ,       UChar * sig) {

	EC_KEY * key = NULL;

	if ((key = EC_KEY_new_by_curve_name(NID_secp256k1)) == NULL)
		throw ec_exception("EC_KEY_new_by_curve_name failed in ec_sign!");

	scope_guard key_guard(std::tr1::bind(EC_KEY_free, key));
	unsigned char dgst[32];
	if (SHA256(msg, len, dgst) == 0)
		throw ec_exception("SHA256 failed in ec_sign!");

	binary_pvt_key_to_key(pvt_key, key);

	unsigned int siglen = 0;
	if (ECDSA_sign(0, dgst, 32, sig, &siglen, key) == 0)
		throw ec_exception("ECDSA_sign failed in ec_sign!");

	return siglen;
}

template<typename UChar>
bool do_verify(const UChar * pub_key, const UChar * msg, const int len,
       const unsigned char * sig, int siglen, unsigned char first_byte = 2) {

	EC_POINT * pub_pt = NULL;
	EC_KEY   * key    = NULL;

	if ((key = EC_KEY_new_by_curve_name(NID_secp256k1)) == NULL)
		throw ec_exception("EC_KEY_new_by_curve_name failed in do_verify!");

	scope_guard     key_guard(std::tr1::bind(EC_KEY_free  , key       ));
	scope_guard  pub_pt_guard(std::tr1::bind(EC_POINT_free, pub_pt    ));

	pub_pt = bn_to_pt(pub_key, key, first_byte);

	EC_KEY_set_public_key( key, pub_pt );

	unsigned char dgst[32];
	SHA256(msg, len, dgst);

	return ECDSA_verify(0, dgst, 32, sig, siglen, key) == 1;
}

template<typename UChar> bool ec_verify(const UChar * pub_key,
              const UChar * msg, int len, const UChar * sig, int siglen) {

	// run both 2 and 3 to prevent a traffic analysis attack
	// based on timing.  You probably wouldn't be able to get
	// the key from the attack, checking the timing can tell
	// an observer if the public key is 2 or 3 which could
	// help identify a peer in situations where we don't want
	// to leak a public key. This would be undesirable if we
	// have passed a public key over an encrypted connection.
	bool good2 = do_verify(pub_key, msg, len, sig, siglen);
	bool good3 = do_verify(pub_key, msg, len, sig, siglen, 3);

	if (good2 || good3)
		return true;

	return false;
}

//#define CRYPTO_DEMO 1
#ifdef CRYPTO_DEMO

void do_print(const unsigned char * data, int len) {
	std::cout << "Len: " << len << " Data: ";
	for (int i = 0; i < len; i++) {
		if (i) std::cout << ":";
		if (data[i] < 16)
			std::cout << "0";
		std::cout << std::hex << (int) data[i];
	} std::cout << std::endl << std::endl;
	std::cout << std::dec;
}

int main() {

	try {

		for(size_t i = 0; i < 10000; i++) {

			if (i % 100 == 0 && i != 0)
				std::cout << i << std::endl;

			unsigned char my_pvt_key[32];
			unsigned char my_pub_key[32];

			unsigned char their_pvt_key[32];
			unsigned char their_pub_key[32];

			generate_ec_keys(    my_pvt_key ,    my_pub_key    );
			generate_ec_keys( their_pvt_key , their_pub_key );

			const unsigned char msg[] = "HELLO THERE FRIEND!";
			unsigned char sig[256];

			int siglen = ec_sign(their_pvt_key, msg, strlen((char *) msg), sig);

			bool do_fail = rand() % 2;

			unsigned char replacement_byte = 0;
			unsigned char replacement_pos  = 0;
			unsigned char original_byte    = 0;

			if (do_fail) {
//				replacement_byte = rand() % 256;
				replacement_byte = 0x80;
				replacement_pos  = rand() % siglen;
				original_byte = sig[replacement_pos];
				if (replacement_byte == sig[replacement_pos])
					do_fail = false;
				sig[replacement_pos] = replacement_byte;
			}

			bool good_sig = ec_verify(their_pub_key, msg, strlen((char *) msg),
			                           sig, siglen);

			if (good_sig == true && do_fail == true) {

				std::cout << "Should have failed due to rewrite "
				               "but it didn't!" << std::endl;
				std::cout << "Original byte: 0x" << std::hex;
				std::cout << (int) original_byte << std::dec << std::endl;
				std::cout << "Replacement byte: 0x" << std::hex;
				std::cout << (int) replacement_byte << std::dec << std::endl;
				std::cout << "Replacement pos: ";
				std::cout << (int) replacement_pos << std::endl;

			} else if (good_sig == false && do_fail == false) {

				std::cout << "Should have succeeded, but failed!" << std::endl;

			}

			unsigned char shared_secret1[32];
			compute_ec_shared_secret(my_pvt_key, their_pub_key, shared_secret1);

			unsigned char shared_secret2[32];
			compute_ec_shared_secret(their_pvt_key, my_pub_key, shared_secret2);

			bool ok = true;

			for (size_t j = 0; j < 32; j++)
				if (shared_secret1[j] != shared_secret2[j])
					ok = false;

			if (!ok)
				std::cerr << "FAILED FAILED FAILED" << std::endl;

		}

	} catch (const std::exception & ex) {
		std::cerr << "std::exception::what(" << ex.what() << ")" << std::endl;
	} catch (...) {
		std::cerr << "Unhandled!" << std::endl;
	}

	return 0;
}

#endif

#endif
