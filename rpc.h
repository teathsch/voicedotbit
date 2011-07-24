// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Make json-rpc calls using curl.

#ifndef RPC_H
#define RPC_H

#include <curl/curl.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>

#include "scopeguard.h"

typedef boost::lock_guard<boost::mutex> mutex_locker;

template<typename CurlPointer> class json_rpc_caller_t {

	public:

		json_rpc_caller_t( const std::string & u    ,
		                   const std::string & p    ,
		                   const std::string & h ,
		                   const std::string & o ) : \
		 curl(NULL) , user(u) , pass(p) , host(h) , port(o) , m() {}

		void SetUser(const std::string & u) { mutex_locker l(m); user = u; }
		void SetPass(const std::string & p) { mutex_locker l(m); pass = p; }
		void SetHost(const std::string & h) { mutex_locker l(m); host = h; }
		void SetPort(const std::string & o) { mutex_locker l(m); port = o; }

		const std::string GetUser() { mutex_locker l(m); return user; }
		const std::string GetPass() { mutex_locker l(m); return pass; }
		const std::string GetHost() { mutex_locker l(m); return host; }
		const std::string GetPort() { mutex_locker l(m); return port; }

		static size_t curl_write_cb( const char        * ptr       ,
		                             const size_t        size      ,
		                             const size_t        nmemb     ,
		                                   std::string * curl_data ) {

			curl_data->append(ptr, nmemb * size);

			return nmemb * size;
		}

		template<typename ParamList>
		const std::string operator()(const std::string & method,
		                             const ParamList   & params) {

			struct curl_slist * curl_headers = NULL;
			scope_guard  g1(std::tr1::bind(curl_slist_free_all, curl_headers));

			mutex_locker      l(m);
			std::string       curl_data;
			char              curl_error_str[CURL_ERROR_SIZE] = { 0 };
			const std::string url = "http:/""/" + host + ":" + port + "/";

			if (curl)
				curl_easy_reset(curl);

			if (!curl)
				curl = curl_easy_init();

			if (!curl)
				throw (const char *) "Couldn't init curl!";

			curl_easy_setopt(curl, CURLOPT_ERRORBUFFER,    curl_error_str   );
//			curl_easy_setopt(curl, CURLOPT_FAILONERROR,    1                );
			curl_easy_setopt(curl, CURLOPT_POST,           1                );
			curl_easy_setopt(curl, CURLOPT_TIMEOUT,        60               );
			curl_easy_setopt(curl, CURLOPT_URL,            url.c_str()      );
			curl_easy_setopt(curl, CURLOPT_WRITEDATA,      &curl_data       );
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
			      json_rpc_caller_t<CurlPointer>::curl_write_cb    );

			if (user.empty() == false && pass.empty() == false) {

				curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
				curl_easy_setopt(curl, CURLOPT_USERPWD , \
				      (user + ":" + pass).c_str());
			}

			// changed jsonrpc to version
			std::string req = "{\"jsonrpc\": "     "1.0, "
			                   "\"id\": "        "\"curltest\", "
			                   "\"method\": "    "\"" + method + "\", "
			                   "\"params\": "    "[ ";

			for (typename ParamList::const_iterator it = params.begin(); \
			       it != params.end(); ++it) {

				if (it != params.begin())
					req += ", ";

				if (method != "sendtoaddress" || it == params.begin()) req += "\"";
				req += *it;
				if (method != "sendtoaddress" || it == params.begin()) req += "\"";
			}
			req += " ] }";

//			std::cout << "JSON IS BETWEEN QUOTES \"" << req << "\"" << std::endl << std::endl;

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS,req.c_str());

			std::stringstream LenStream;
			LenStream << "Content-Length: " << req.length();
			curl_headers = curl_slist_append(curl_headers,LenStream.str().c_str());

			curl_headers = curl_slist_append(curl_headers,
			 "Content-Type: application/json");

			curl_headers = curl_slist_append(curl_headers,
			 "User-Agent: curl");

			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);

			if (curl_easy_perform(curl)) {
				std::cout << "The curl data is: " << curl_data << std::endl;
//				curl_data = "";
			}

			long int http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
//			if (http_code == 200 && curl_code != CURLE_ABORTED_BY_CALLBACK)
//{
         //Succeeded
//}

			if (http_code > 299) {
				throw (const char *) curl_data.c_str();
//			if (strlen(curl_error_str)) {
//				std::string err;
//				err.append(curl_error_str, strlen(curl_error_str));
//				throw (const char *) err.c_str();
			}

			return curl_data;
		}

	private:
		CurlPointer curl;
		std::string user;
		std::string pass;
		std::string host;
		std::string port;
		boost::mutex m;

}; typedef json_rpc_caller_t<CURL *> json_rpc_caller;

#endif
