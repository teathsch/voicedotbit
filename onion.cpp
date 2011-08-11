// test onion stuff

#include "onion.h"


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
