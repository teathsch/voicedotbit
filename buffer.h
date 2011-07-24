// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Provides a simple, efficient, fixed size buffer of max ipv4 udp size

#ifndef MYBUFFER_H
#define MYBUFFER_H

#include <iostream>
#include <stdint.h>
#include <cstring>
#include <string>

template<size_t max_size> class buffer_t {
	public:

		buffer_t(const char * tempdata, size_t templen) : data() , \
		          len(templen) , iter(0) {
			if (len > max_size)
				throw (const char *) ("Overflow.");
			memcpy(data, tempdata, len);
		}

		buffer_t(const char * rhs) : data() , len(0) , iter(0) {
			*this = rhs;
		}

		buffer_t & operator = (const char * rhs) {
			len = strlen((char *) rhs);
			if (len > max_size)
				throw (const char *) ("Overflow.");
			memcpy(data, (int8_t *) rhs, len);
			iter = 0;
			return *this;
		}

		buffer_t() : data() , len(0) , iter(0) { memset(data, 0, max_size); }

		buffer_t(const buffer_t<max_size> & rhs) : \
		               data() , len(rhs.len) , iter(rhs.iter) {
			*this = rhs;
		}


		buffer_t & operator = (const buffer_t<max_size> & rhs) {
			len = rhs.len;
			if (len > max_size)
				throw (const int8_t *) ("Too big!");
			memset((int8_t *) data, 0, max_size);
			memcpy((int8_t *) data, rhs.data, len);
			return *this;
		}


		const buffer_t operator + (const buffer_t & rhs) {

			buffer_t<max_size> temp(*this);

			if (temp.len + rhs.len > max_size)
				throw (const char *) ("Overflow.");

			memcpy((int8_t *) (temp.data + temp.len), (int8_t *) rhs.data, rhs.len);
			temp.len += rhs.len;
			return temp;
		}

		const char * operator * () const {return (const char *) data; }
		char * operator * () { return (char *) data; }

		const uint8_t & operator[](const size_t & idx) const {
			if (idx > max_size)
				throw (const char *) ("Out of range!");
			return data[idx];
		}

		uint8_t & operator[](const size_t & idx) {
			if (idx > max_size)
				throw (const char *) ("Out of range!");
			return data[idx];
		}

		~buffer_t() {}

		const size_t & length()   const { return len;  }
		const size_t & iterator() const { return iter; }

		template<size_t bufsize>
		friend std::ostream & operator << (std::ostream &,const buffer_t &);

		buffer_t & operator << (const std::string & rhs) {

			if (rhs.length() > max_size)
				throw (const char *) ("Too big.");

			*this << (uint16_t) rhs.length();
			for (size_t i = 0; i < rhs.length(); i++)
				*this << rhs[i];
			return *this;
		}

		buffer_t & operator >> (std::string & rhs) {
			uint16_t len;
			*this >> len;
			char temp[max_size];
			memset(temp, 0, max_size);
			memcpy(temp, data + iter, len);
			rhs = temp;
			iter += len;
			return *this;
		}

		template<typename T> buffer_t & operator << (const T & rhs) {

			if (sizeof(rhs) > max_size)
				throw (const char *) ("Too big.");

			memcpy(data + len, (char *) (&rhs), sizeof(rhs));
			len += sizeof(rhs);
			return *this;
		}

		template<typename T> buffer_t & operator >> (T & rhs) {
			if (sizeof(T) + iter > len)
				throw (const char *) ("Out of range.");
			memcpy(&rhs, data + iter, sizeof(T));
			iter += sizeof(T);
			return *this;
		}

		void begin() { iter = 0; };

		const bool operator < (const buffer_t & rhs) {
			return strncmp((char *) this->data, (char *) rhs.data, \
			    this->len < rhs.len ? this->len : rhs.len) < 0;
		}

		const bool operator ==(const buffer_t & rhs) {
			return strncmp(this->data, rhs.data, \
			    this->len < rhs.len ? this->len : rhs.len) == 0;
		}

		template<size_t num_bytes>
		buffer_t & read_from_istream(std::istream & fin) {
			if (num_bytes > max_size)
				throw (const char *) ("Too big.");
			iter = 0;
			char temp[num_bytes];
			fin.read(temp, num_bytes);
			len = fin.gcount();
			memcpy(data, temp, len);
			return *this;
	}

	private:
		unsigned char data[max_size];
		size_t len;
		size_t iter;
};

template<size_t bufsize>
std::ostream & operator << (std::ostream & o, const buffer_t<bufsize> & b) {
	for (size_t i = 0; i < b.length(); i++)
		o << b[i] << std::flush;
	return o;
}

typedef buffer_t<65535> buffer;

#endif
