// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// create and parse onion packets for setting up anonymous routes

#ifndef ONION_H
#define ONION_H

#include <iostream>
#include "buffer.h"
#include "ecdh_aes.h"
#include "base64.h"
#include <queue>
#include <set>

using std::set;
using std::deque;

void print_bytes(unsigned char * p, size_t num) {
	for (size_t i = 0; i < num; i++) {
		std::cout << i << ":" << (int) p[i] << " ";
	} std::cout << std::endl << std::endl;
}

using std::string;

template<typename UChar>
int add_layer( UChar      * onion   , uint16_t   onion_len   ,
               const char * name    , uint16_t   name_len    ,
               const char * host    , uint16_t   host_len    ,
               uint16_t     port    , UChar    * pub_key     ) {

	unsigned char new_onion[65535];
	unsigned char * pnew_onion = new_onion;
	uint16_t new_onion_len = 0;

//	std::cout << "add_layer onion_len: " << onion_len << std::endl;

	memcpy(pnew_onion, (char *) &onion_len, sizeof(onion_len));
	pnew_onion += sizeof(onion_len);
	new_onion_len += sizeof(onion_len);

	memcpy(pnew_onion, (char *) onion, onion_len);
	pnew_onion += onion_len;
	new_onion_len += onion_len;




	//uint16_t templen = host_len;
	memcpy(pnew_onion, (char *) &name_len, sizeof(name_len));
	pnew_onion += sizeof(name_len);
	new_onion_len += sizeof(name_len);

	memcpy(pnew_onion, (char *) name, name_len);
	pnew_onion += name_len;
	new_onion_len += name_len;




	uint16_t templen = host_len;
	memcpy(pnew_onion, (char *) &templen, sizeof(templen));
	pnew_onion += sizeof(templen);
	new_onion_len += sizeof(templen);

	memcpy(pnew_onion, (char *) host, host_len);
	pnew_onion += host_len;
	new_onion_len += host_len;
//	memcpy(pnew_onion, (char *) host.c_str(), host.length());
//	pnew_onion += host.length();
//	new_onion_len += host.length();

	memcpy(pnew_onion, (char *) &port, sizeof(port));
	pnew_onion += sizeof(port);
	new_onion_len += sizeof(port);

	if (pub_key == NULL) {
		memcpy(onion, new_onion, new_onion_len);
		return new_onion_len;
	}

	return encrypt_once(pub_key, new_onion, new_onion_len, onion);
}

template<typename UChar>
int remove_layer(UChar * onion, uint16_t onion_len, UChar * prev_onion, unsigned char * pvt_key) {

	//std::cout << "bytes to parse" << std::endl;
	//print_bytes(onion, onion_len);

	unsigned char decrypted[65535];

	//std::cout << "The onion len: " << onion_len << std::endl;

	if (pvt_key == NULL) {
		//std::cout << "not decrypting" << std::endl;
		memcpy(decrypted, onion, onion_len);
		//print_bytes(decrypted, onion_len);
//		std::cout << "dumping nicely" << std::endl;
//		for (int i = 0; i < onion_len; i++) {
//			if ((decrypted[i] >= 'A' && decrypted[i] <= 'Z') || \
//			    (decrypted[i] >= 'a' && decrypted[i] <= 'z') || \
//			    (decrypted[i] >= '0' && decrypted[i] <= '9') || \
//			     decrypted[i] == '.') {
//				std::cout << i << ":" << decrypted[i] << " ";
//			}
//		} std::cout << std::endl << std::endl;
	} else {
		//std::cout << "decrypting" << std::endl;
		decrypt_once(pvt_key, onion, onion_len, decrypted);
	}

	unsigned char * ponion = decrypted;

	uint16_t prev_onion_len = 0;
	memcpy((char *) &prev_onion_len, ponion, sizeof(prev_onion_len));
	ponion += sizeof(prev_onion_len);

//	std::cout << "The vars..." << std::endl;

//	std::cout << "prev_onion_len: " << prev_onion_len << std::endl;

//	unsigned char prev_onion[65535];
	memcpy((char *) prev_onion, ponion, prev_onion_len);
	ponion += prev_onion_len;


	uint16_t name_len;
	memcpy((char *) &name_len, ponion, sizeof(name_len));
	ponion += sizeof(name_len);

	std::string name;
	name.append((char *) ponion, name_len);
	ponion += name_len;

	std::cout << "name: " << name << std::endl;


	uint16_t host_len;
	memcpy((char *) &host_len, ponion, sizeof(host_len));
	ponion += sizeof(host_len);

	std::string host;
	host.append((char *) ponion, host_len);
	ponion += host_len;

	std::cout << "host: " << host << std::endl;


	uint16_t port;
	memcpy((char *) &port, ponion, sizeof(port));
	ponion += sizeof(port);

	std::cout << "port: " << port << std::endl;

//	memcpy(onion, prev_onion, prev_onion_len);

	return prev_onion_len;
}

template<typename UChar, typename ListType>
int make_onion_path(UChar * onion_path, /* UChar * pub_key, */ ListType & relays) {

	int onion_path_len = 0;

	for (typename ListType::iterator it = relays.begin(); it != relays.end(); ++it) {

		unsigned char * pub_key = NULL;

		if (it + 1 != relays.end())
			pub_key = it->get_pub_key();

		char name[255];
		memset(name, 0, 255);
		uint16_t name_len = 0;

		char host[255];
		memset(host, 0, 255);
		uint16_t host_len = 0;

		uint16_t port = 0; // should be the final port

		if (it != relays.begin()) {
			memcpy(host, it->get_host(), it->get_host_len());
			host_len = it->get_host_len();

//			std::cout << "Setting name to: " << it->get_name() << std::endl;

			memcpy(name, it->get_name().c_str(), it->get_name().length());
			name_len = it->get_name().length();
			port = it->get_port();
		} else {
			char send_to_session[] = "session:";

//			std::cout << "Not setting name" << std::endl;

			memcpy(name, send_to_session, 8);
			name_len = 0;
			memcpy(host, send_to_session, 8);
			host_len = 0;
			port = 0; // should be the final port
		}

//		unsigned char * pub_key = it->get_pub_key();
//		if (it + 1 == relays.end())

		onion_path_len = \
		 add_layer( onion_path , onion_path_len , name , name_len, host ,
		            host_len   , port           , pub_key );

		//std::cout << "Bytes after add layer..." << std::endl;
		//print_bytes(onion_path, onion_path_len);

//		 it->get_host() ,
//		            it->get_host_len() , it->get_port() , pub_key        );
	}

	return onion_path_len;
}

template<typename SetType>
uint16_t make_onion( unsigned char * onion               ,
                     unsigned char * pvt_signing_key     ,
                     set<SetType>  & relays              ,
                     const size_t  & relays_per_path = 3 ,
                     const size_t  & num_paths       = 3 ) {

	uint16_t onion_len = 0;

	for (size_t i = 0; i < num_paths; i++) {

		deque<SetType> path_relays;

		for (size_t j = 0; j < relays_per_path; j++) {
			path_relays.push_back(*(relays.begin()));
		}

		uint16_t templen = make_onion_path(onion + onion_len + 2, path_relays);

		memcpy(onion + onion_len, (char *) &templen, sizeof(templen));
		onion_len += templen + sizeof(templen);
	}

	onion[onion_len + 1] = 0;
	onion[onion_len + 2] = 0;
	onion_len += 2;

	uint16_t sig_len = ec_sign(pvt_signing_key, onion, onion_len, onion + onion_len);

	return onion_len + sig_len;
}

template<typename UChar>
int count_onion_paths(UChar * onion, int onion_len, UChar * pubkey /* , UChar * pvtkey */) {

	for (int pos = -2, len = 1; len != 0; pos += len + 2) {
		len = *((uint16_t *) (onion + (pos + 2)));
		if (len == 0) {
			int verify_res = ec_verify(pubkey, onion, pos + 4, onion + pos + 4, onion_len - pos + 4);
			std::cout << "verify res: " << verify_res << std::endl;
		} else {

			unsigned char temp_onion[65535];
			memcpy(temp_onion, onion + pos + 4, len);

			unsigned char prev_onion[65535];

			remove_layer(temp_onion, len, prev_onion, NULL);

// DONT DO THIS CUZ IT SCREWS UP VERIFY.. COPY onion + pos + 4, len to a new buf first
//			remove_layer(onion + pos + 4, len, onion, pvtkey);
//int remove_layer(UChar * onion, uint16_t onion_len, UChar * pvt_key, UChar *

			// REMOVE LAYERS HERE
		}
	}

	return 0;
}

template<typename T> class relay_t {

	public:
		relay_t() : name() , host() , host_len(0) , port(0) , pub_key() , pvt_key() {
			memset(host, 0, 255);
			memset(pub_key, 0, 32 );
			memset(pvt_key, 0, 32 );
		}

		relay_t(const relay_t<T> & rhs) : name(rhs.name) , host() ,
		 host_len(rhs.host_len) , port(rhs.port) , pub_key() , pvt_key() {
			memset(host, 0, 255);
			memcpy(host, rhs.host, rhs.host_len);
			memcpy(pub_key, rhs.pub_key, 32);
			memcpy(pvt_key, rhs.pvt_key, 32);
		}

		// for std::set<T>::find
		relay_t(string tmpname) : name(tmpname) , host() , \
		  host_len(0) , port(0) , pub_key() , pvt_key() {
			memset(host   , 0, 255);
			memset(pub_key, 0, 32 );
			memset(pvt_key, 0, 32 );
		}

		relay_t(string tmpname, const char * temp_host,
		 const int & temp_host_len, const uint16_t & temp_port,
		 const unsigned char * temp_pub_key,
		 const unsigned char * temp_pvt_key = NULL) :
		 name(tmpname) , host() , host_len(temp_host_len) ,
		 port(temp_port) , pub_key() , pvt_key() {
			memset(host, 0, 255);
			memcpy(host, temp_host, temp_host_len);
			memcpy(pub_key, temp_pub_key, 32);
			if (temp_pvt_key == NULL)
				memset(pvt_key, 0, 32);
			else
				memcpy(pvt_key, temp_pvt_key, 32);
		}

		~relay_t() {}

		const string & get_name() const { return name; }
		string & get_name() { return name; }

		const unsigned char * get_pub_key() const { return pub_key; }
		unsigned char * get_pub_key() { return pub_key; }

		const unsigned char * get_pvt_key() const { return pvt_key; }
		unsigned char * get_pvt_key() { return pvt_key; }

		const char * get_host() const { return host; }
		char * get_host() { return host; }

		const int & get_host_len() const { return host_len; }
		int & get_host_len() { return host_len; }

		const uint16_t & get_port() const { return port; }
		uint16_t & get_port() { return port; }

		bool operator< (const relay_t<T> & rhs) const {
			return this->name < rhs.name;
			//return strncmp(host, rhs.host, std::min(host_len, rhs.host_len));
		}

		bool operator == (const relay_t<T> & rhs) const {
			return this->name = rhs.name;
		}

		//bool operator == (const string & rhs) const {
		//	return name == rhs;
		//}

	private:
		relay_t & operator = (const relay_t<T> & rhs) {}
		string name;
		char host[255];
		int host_len;
		uint16_t port;
		unsigned char pub_key[32];
		unsigned char pvt_key[32];

}; typedef relay_t<int> relay;

/*
int main() {

	for (size_t i = 0; i < 1; i++) {

		try {

			std::set<relay> relays;

//unsigned char temp_pvt_encryption_key[32];
  //          unsigned char temp_pub_encryption_key[32];

    //        generate_ec_keys(temp_pvt_encryption_key, temp_pub_encryption_key);

			std::deque<string> relay_names;
			relay_names.push_back("d/me");
			relay_names.push_back("d/you");
			relay_names.push_back("d/them");
			relay_names.push_back("d/us");
			relay_names.push_back("d/who");
			relay_names.push_back("d/what");
			relay_names.push_back("d/when");
			relay_names.push_back("d/where");
			relay_names.push_back("d/why");

			for (size_t i = 0; i < 9; i++) {

				unsigned char temp_pvt_encryption_key[32];
				unsigned char temp_pub_encryption_key[32];

				generate_ec_keys(temp_pvt_encryption_key, temp_pub_encryption_key);

				string host = "127.0.0.1";
				int port = 9654;
//				string name = "d/whatever";
				string name = relay_names.at(i);

				relays.insert(relay(name, host.c_str(), host.length(), port,
				 temp_pub_encryption_key, temp_pvt_encryption_key));
				//relays.insert(relay(temp_pub_encryption_key, host.c_str(), host.length(), 9654));

				//unsigned char pvt_signing_key[32];
				//unsigned char pub_signing_key[32];

			}

			//(relays.find(string("d/whatever")))

			unsigned char onion[65535];

//			unsigned char pvt_encryption_key[32];
//			unsigned char pub_encryption_key[32];

			unsigned char pvt_signing_key[32];
			unsigned char pub_signing_key[32];

//			generate_ec_keys(pvt_encryption_key, pub_encryption_key);

//			std::cout << "pvt_encryption_key: " << std::endl;
//			print_bytes(pvt_encryption_key, 32);

			generate_ec_keys(pvt_signing_key, pub_signing_key);

			int onion_len = make_onion(onion, pvt_signing_key, relays);

			count_onion_paths(onion, onion_len, pub_signing_key);

			std::cout << "onion_len: " << onion_len << std::endl;

//			count_onion_paths(onion, onion_len, pub_signing_key, pvt_encryption_key);

		} catch (const char * ex) {
			std::cerr << "const char *: " << ex << std::endl;
		}

	}

	return 0;

}
*/

#endif
