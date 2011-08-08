// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// asio onion server class

#ifndef ONIONSERVER_H
#define ONIONSERVER_H

#include "server.h"
//#include "cryptopeer.h"

#include <map>

using std::map;
using std::string;

template<typename IO> class onion_server_t : public server<> {

	public:
		onion_server_t(IO & io, int port) : server<>(io, port) {}

		void handle_data(unsigned char * buf, size_t len) {

			buf = NULL; // suppress warning for now
			len = 0; // suppress warning for now

//			std::cout << "sender host: " << get_sender_host() << std::endl;
//			std::cout << "sender port: " << get_sender_port() << std::endl;

//			this->send_back(buf, len);

		}

		bool handle_error() { return true; }

	private:
//		map<string, crypto_peer> peers;
//		map<string, crypto_peer> users;

}; typedef onion_server_t<asio::io_service> onion_server;

#endif
