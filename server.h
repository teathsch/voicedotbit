// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// asio server class

#ifndef SERVER_H
#define SERVER_H

#include <asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <tr1/functional>
#include <iostream>

using asio::ip::udp;

template<size_t = 0> class server {
	public:

		server(asio::io_service & tempio, int port) : \
		 io(tempio) , \
		 socket_(tempio, udp::endpoint(udp::v4(), port)) {

			socket_.async_receive_from(
		    asio::buffer(data_, max_length), sender_endpoint_,
		    boost::bind(&server::handle_receive, this,
		    asio::placeholders::error,
		    asio::placeholders::bytes_transferred));
		}

		virtual ~server() {}

		virtual void handle_data(unsigned char *, size_t) = 0;
		virtual bool handle_error() = 0;

		virtual const std::string get_sender_host() {
			return sender_endpoint_.address().to_string();
		}

		int get_sender_port() {
			return sender_endpoint_.port();
		}

		void send(unsigned char * data, int len, const udp::endpoint & endpoint) {
			socket_.async_send_to(
			 asio::buffer((char *) data, len), endpoint,
			 boost::bind(&server::handle_send, this,
			   asio::placeholders::error,
			   asio::placeholders::bytes_transferred));
		}

		inline void send_back(unsigned char * data, int len) {
			this->send(data, len, this->sender_endpoint_);
		}

		void handle_send(const asio::error_code& error, size_t bytes_sent) {
			if (error)
				this->handle_error();
		}

		void handle_receive(const asio::error_code& error, size_t bytes_recvd) {

			if (handle_error() == false)
				return;

			this->handle_data((unsigned char *) data_, bytes_recvd);

			socket_.async_receive_from(
			 asio::buffer(data_, max_length), sender_endpoint_,
			 boost::bind(&server::handle_receive, this,
			 asio::placeholders::error,
			 asio::placeholders::bytes_transferred));

		}

	protected:
		asio::io_service & io;
		udp::socket socket_;

		udp::endpoint sender_endpoint_;
		enum { max_length = 65536 };
		char data_[max_length];
};

#endif
