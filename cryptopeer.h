#ifndef CRYPTOPEER_H
#define CRYPTOPEER_H

#include "myaes.h"
#include "curve25519.h"
#include "buffer.h"

#include <deque>
using std::deque;

/*

 Requirements
 ============

 Alice needs to authenticate bob.
 Bob doesn't need to authenticate Alice.
 They want perfect forward secrecy.
 Bob has already published a pvt/pub ecdh keypair. Alice has it.

 Method
 ======

 Alice generates a pvt/pub ecdh keypair
 Alice generates a temp AES key from her ecdh keys + Bob's pub key
 Alice generates a nonce and encrypts it with the AES key
 Alice sends the encrypted nonce and her cleartext pub key
 Bob generates the temp AES key with his priv key and Alice's pub ecdh key
 Bob decrypts the nonce with the temp AES key
 Bob generates an ephemeral pvt/pub ecdh keypair
 Bob computes an ephemeral AES key from the ephemeral keys + Alice's pub ecdh key
 Bob sends AES(ephemeral aes, nonce xor ephemeral ecdh pub) along with his ephemeral pub ecdh key
 Alice generates the ephemeral AES key using her ecdh keys + Bob's ephemeral ecdh key
 Alice verifies that nonce = AES_DECRYPT(ephemeral aes, Bob's nonce) xor ephemeral ecdh pub
*/

template<size_t = 0> class crypto_friend {

	public:

		crypto_friend() : data() , data_len(0) , shared() {

			memset(data,   0, 256);
			memset(shared, 0, 32 );
		}

		virtual ~crypto_friend() {}

		int get_data(unsigned char * buf) {

			memcpy(buf, data, data_len);
			return data_len;
		}

		crypto_friend(const crypto_friend<> & rhs) : \
		 data() , data_len(rhs.data_len) , shared() {

			memcpy(this->data, rhs.data, rhs.data_len);
			memcpy(this->shared, rhs.shared, 32);
		}

	protected:

		unsigned char data[256];
		int data_len;
		unsigned char shared[32];
		deque<buffer> packet_queue;
};

template<size_t = 0> class crypto_user_t : public crypto_friend<> {
	public:

		crypto_user_t(const crypto_user_t<> & rhs) : crypto_friend<>(rhs) , \
		 ecdh_pvt() , nonce() {

			memcpy(this->ecdh_pvt, rhs.ecdh_pvt, 32);
			memcpy(this->nonce   , rhs.nonce   , 32);
		}

		crypto_user_t(unsigned char * their_pubkey) : crypto_friend<>() , \
		 ecdh_pvt() , nonce() {

			unsigned char ecdh_pub[32];
			unsigned char temp_shared[32];
			unsigned char encrypted_nonce[256];

			memset(ecdh_pub,        0, 32 );
			memset(temp_shared,     0, 32 );
			memset(encrypted_nonce, 0, 256);

			curve25519_keypairgen(ecdh_pvt, ecdh_pub);
			curve25519(temp_shared, ecdh_pvt, their_pubkey);

			for (size_t i = 0; i < 32; i++)
				nonce[i] = rand() % 256;

			int encrypted_nonce_len = \
			 aes_encrypt(temp_shared, nonce, 32, encrypted_nonce);

			memcpy(data, ecdh_pub, 32);
			memcpy(data + 32, encrypted_nonce, encrypted_nonce_len);
			data_len = 32 + encrypted_nonce_len;
		}

		bool verify(unsigned char * response, int response_len) {

			unsigned char decrypted_nonce[256];
			bool success = true;
			int decrypted_nonce_len = 0;

			memset(decrypted_nonce, 0, 256);

			curve25519(shared, ecdh_pvt, response);

			decrypted_nonce_len = aes_decrypt(shared, response + 32, response_len - 32, decrypted_nonce);

			for (size_t i = 0; i < 32; i++) {

				if ((nonce[i] ^ response[i]) != decrypted_nonce[i]) {

					success = false;
					break;
				}
			}
			return success;
		}

		const unsigned char * get_shared() const { return shared; }

	private:

		unsigned char ecdh_pvt[32];
		unsigned char nonce[32];

}; typedef crypto_user_t<> crypto_user;

template<size_t = 0> class crypto_peer_t : public crypto_friend<> {

	public:

		crypto_peer_t(const crypto_peer_t<> & rhs) : crypto_friend<>(rhs) {}

		crypto_peer_t( unsigned char * my_pvtkey  , \
		               unsigned char * intro      , \
		               int             intro_len) : crypto_friend<>() {

			unsigned char temp_shared[32];
			unsigned char decrypted_nonce[32];
			unsigned char ecdh_pvt[32];
			unsigned char ecdh_pub[32];
			unsigned char encrypted_nonce[256];

			memset(temp_shared,     0, 32 );
			memset(decrypted_nonce, 0, 32 );
			memset(ecdh_pvt,        0, 32 );
			memset(ecdh_pub,        0, 32 );
			memset(encrypted_nonce, 0, 256);

			// the first part of the intro is the pubkey
			curve25519(temp_shared, my_pvtkey, intro);

			aes_decrypt(temp_shared, intro + 32, intro_len - 32, decrypted_nonce);

			curve25519_keypairgen(ecdh_pvt, ecdh_pub);
			curve25519(shared, ecdh_pvt, intro);

			unsigned char nonce_xor_pubkey[32];

			for (size_t i = 0; i < 32; i++)
				nonce_xor_pubkey[i] = decrypted_nonce[i] ^ ecdh_pub[i];

			int encrypted_nonce_len = \
			 aes_encrypt(shared, nonce_xor_pubkey, 32, encrypted_nonce);

			memcpy(data, ecdh_pub, 32);
			memcpy(data + 32, encrypted_nonce, encrypted_nonce_len);

			data_len = 32 + encrypted_nonce_len;

		}

		const unsigned char * get_shared() const { return shared; }

}; typedef crypto_peer_t<> crypto_peer;

//#define EXAMPLE_IMPL 1
#ifdef  EXAMPLE_IMPL

#include <iostream>

using std::cout;
using std::endl;

void do_print(std::string label, const unsigned char * buf, int len) {

   cout << label << ": ";

   for (int i = 0; i < len; i++) {
      cout << std::hex << (int) buf[i];
   }

	cout << endl << endl;
}

int main() {

	srand(time(NULL));

	unsigned char ecdh_pub[32];
	unsigned char ecdh_pvt[32];
	curve25519_keypairgen(ecdh_pvt, ecdh_pub);

	crypto_user user(ecdh_pub);
	crypto_user user2(user);

	unsigned char intro[256];
	int intro_len = user.get_data(intro);

	crypto_peer peer(ecdh_pvt, intro, intro_len);

	unsigned char response[256];
	int response_len = peer.get_data(response);

	cout << "\nResult: " << user.verify(response, response_len) << endl << endl;

	do_print("shared", user.get_shared(), 32);
	do_print("shared", peer.get_shared(), 32);

	return 0;

}

#endif

#endif
