// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// asio server class

#ifndef APPSERVER_H
#define APPSERVER_H

#include "server.h"

template<typename IO> class app_server_t : public server<> {

	public:
		app_server_t(IO & io, int port) : server<>(io, port) {}

		void handle_data(unsigned char * buf, size_t len) {

			buf = NULL; // suppress warning for now
			len = 0; // suppress warning for now

//			std::cout << "sender host: " << get_sender_host() << std::endl;
//			std::cout << "sender port: " << get_sender_port() << std::endl;

//			this->send_back(buf, len);

		}

		bool handle_error() { return true; }

}; typedef app_server_t<asio::io_service> app_server;

#endif
