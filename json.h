// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Parse namecoin specific json data to provide API.

#ifndef JSON_H
#define JSON_H

#include "json_spirit_v4.03/json_spirit/json_spirit_reader_template.h"
#include "json_spirit_v4.03/json_spirit/json_spirit_writer_template.h"
#include "rpc.h"

using json_spirit::Value;
using json_spirit::Array;
using json_spirit::Object;
using json_spirit::Pair;
using std::string;
using std::vector;
using std::map;

template<typename StringType> class json_unexpected_t {
	public:
		json_unexpected_t(const StringType & wh) : itsWhat(wh) {}
		const StringType what() { return itsWhat; }
	private:
		const StringType itsWhat;
}; typedef json_unexpected_t<string> json_unexpected;

class json_interface {

	public:

		json_interface(const string & user, \
		               const string & pass, \
		               const string & host, \
		               const string & port) : \
		 json(user, pass, host, port) {}

		const bool SetUser(const string & user) { json.SetUser(user); return this->check(); }
		const bool SetPass(const string & pass) { json.SetPass(pass); return this->check(); }
		const bool SetHost(const string & host) { json.SetHost(host); return this->check(); }
		const bool SetPort(const string & port) { json.SetPort(port); return this->check(); }

		const bool check() {
			return true; // FIXME: check to see if new credents are valid
		}

		const string GetUser() { return json.GetUser(); }
		const string GetPass() { return json.GetPass(); }
		const string GetHost() { return json.GetHost(); }
		const string GetPort() { return json.GetPort(); }

		Value get_json(const string & method, const vector<string> & params) {

			std::stringstream ss;
			string resstr = json(method, params);
			ss << resstr;
			Value val;
			json_spirit::read_stream(ss, val);

			Object obj(val.get_obj());

			Value res;

			size_t validParts = 0;

			for (size_t i = 0; i < obj.size(); ++i) {

				Pair p(obj[i]);

				if (p.name_ == "error") {
					std::stringstream errstream;
					json_spirit::write_stream(p.value_, errstream, true);
					string errtext;
					std::getline(errstream, errtext, '\0');
					if (errtext.empty() == false && errtext != "null")
						throw (const char *) errtext.c_str();
					validParts++;
				} else if (p.name_ == "result") {
					res = p.value_;
					validParts++;
				}
			}

			if (validParts < 2)
				throw (const char *) "Didn't get complete json response!";

			return res;
		}

		void deletetransaction(const string & txid) {

			vector<string> params;
			params.push_back(txid);
			get_json("deletetransaction", params);
		}

		const bool validateaddress(const string & address) {
			vector<string> params;
			params.push_back(address);
			Value val(get_json("validateaddress", params));

			Object obj(val.get_obj());

			bool isvalid = false;

			for (size_t i = 0; i < obj.size(); i++)
				if (obj[i].name_ == "isvalid" && obj[i].value_.get_bool() == true)
					isvalid = true;

			return isvalid;
		}

		void transfer_name(const string & address, const string & name) {

			vector<string> params;
			params.push_back(name);
			params.push_back("");
			params.push_back(address);
			get_json("name_update", params);
		}

		const string escape(const string & temp) {
			string res;
			for (size_t i = 0; i < temp.length(); i++) {
				if (temp[i] == '"') {
					res += "\\\"";
				} else {
					res += temp[i];
				}
			}
			return res;
		}

		const int name_update(const string & name, const string & theval) {
			vector<string> params;
			params.push_back(name);
			params.push_back(this->escape(theval));
			get_json("name_update", params);
		}

		const int name_firstupdate(const string & name, string long_hash, string short_hash, const string & thevalue) {
			vector<string> params;
			params.push_back(name);
			params.push_back(long_hash);
			params.push_back(this->escape(thevalue));

			try {
				get_json("name_firstupdate", params);
			} catch (...) {

				std::cout << "Failed with long hash... trying with short hash..." << std::endl;

				params.at(1) = short_hash;

				try {

					get_json("name_firstupdate", params);

				} catch (...) {
					return 1;
				}

				//return 1;

			}

			return 0;

		}

		const int name_new(const string & name, string & long_hash, string & short_hash) {

			try {

				vector<string> params;
				params.push_back(name);

				Value val(get_json("name_new", params));

				Array arr(val.get_array());

				if (arr.size() < 2) {
					return 3; // not enough hashes
				}

				long_hash = arr[0].get_str();
				short_hash  = arr[1].get_str();

				//hash1 = arr[0].get_str();
				//hash2 = arr[1].get_str();

			} catch (...) {
				return 2;
			}

			return 0;

		}

		const int name_new_and_firstupdate(const string & name, string theval = "") {

			vector<string> params;
			params.push_back(name);
//			Value val(get_json("name_new", params));

			string hash1, hash2;

			try {
				Value val(get_json("name_new", params));

				Array arr(val.get_array());

				if (arr.size() < 2) {
					return 3; // not enough hashes
				}

				hash1 = arr[0].get_str();
				hash2 = arr[1].get_str();

			} catch (const std::exception & ex) {

				std::cout << "Runtime Exception: " << ex.what() << std::endl;
				return 2; // name_new failed

			} catch (const char * ex) {

				std::cout << "const char *: " << ex << std::endl;
				return 2;

			} catch (...) {
				return 2; // name_new faild
			}

			params.push_back(hash2);
			params.push_back(this->escape(theval));

			try {
				Value val(get_json("name_firstupdate", params));
			} catch (...) {
				return 1; //name_firstupdate failed
			}

			return 0;

//			name_firstupdate d/myname 0987654321 '{"map":{"":"1.2.3.4"}}'

		}

		template<typename T> string to_str(T t) {
			std::stringstream ss;
			ss << t;
			return ss.str();
		}

		const bool listtransactions(vector<map<string, string> > & res) {

			Value val(get_json("listtransactions", vector<string>()));

			Array arr(val.get_array());

			for (size_t i = 0; i < arr.size(); i++) {

				Object obj(arr[i].get_obj());

				for (size_t j = 0; j < obj.size(); j++) {


					if (obj[j].name_ == "account")
						res.push_back(map<string, string>());

					if (obj[j].name_ == "amount" || \
					    obj[j].name_ == "fee") {
						res.back()[obj[j].name_] = to_str(obj[j].value_.get_real());
					} else if (obj[j].name_ == "time"          || \
					           obj[j].name_ == "confirmations"  ) {
						res.back()[obj[j].name_] = to_str(obj[j].value_.get_int());
					} else {
						res.back()[obj[j].name_] = obj[j].value_.get_str();
					}

				}

			}

		}

		const bool getaddressesbyaccount(const string & account, vector<string> & addresses) {

			if (addresses.size())
				addresses.clear();

			vector<string> params;
			params.push_back(account);
			Value val(get_json("getaddressesbyaccount", params));

			Array arr(val.get_array());

			for (size_t i = 0; i < arr.size(); i++)
				addresses.push_back(arr[i].get_str());

			return true;

		}

		void send_to_address(const string & address, const string & amount) {

			vector<string> params;

			std::cout << "Sending..." << std::endl;
			std::cout << "Address: " << address << std::endl;
			std::cout << "Amount: " << amount << std::endl << std::endl;

			params.push_back(address);
			params.push_back(amount);
			get_json("sendtoaddress", params);
		}

		double get_balance() {
			Value val(get_json("getbalance", vector<string>()));
			return val.get_real();
		}

		int getblockcount() {
			Value val(get_json("getblockcount", vector<string>()));
			return val.get_int();
		}

		int getconnectioncount() {
			Value val(get_json("getconnectioncount", vector<string>()));
			return val.get_int();
		}

		double calculate_fee() {

			int nBlock = getblockcount();

		   if (nBlock >= 24000) nBlock += (nBlock - 24000) * 3;

		   int res = 500000000 >> (int) floor(nBlock / 8192);
		   res = res - (res >> 14) * (nBlock % 8192);

		   return (double) res / 10000000;

		}

		void getinfo(map<string, string> & mapres) {
			Value val(get_json("getinfo", vector<string>()));
			Object obj(val.get_obj());

			for (size_t i = 0; i < obj.size(); i++) {
				string name = obj[i].name_;
				string val;
				       if (name == "version"      || name == "blocks"       || \
				           name == "connections"  || name == "hashespersec" || \
				           name == "genproclimit" || name == "keypoololdest" ) {
					val = to_str(obj[i].value_.get_int());
				} else if (name == "balance"      || name == "difficulty"   || \
				           name == "paytxfee"     ) {
					val = to_str(obj[i].value_.get_real());
				} else if (name == "generate"     || name == "testnet"       ) {
					val = to_str(obj[i].value_.get_bool());
				} else if (name == "proxy"        || name == "errors"        ) {
					val = obj[i].value_.get_str();
				}

				mapres.insert(std::make_pair(name, val));

			}

		}

		const string name_get_value(const string & tempname) {

			Value val(get_json("name_list", vector<string>()));
			Array arr(val.get_array());

			for (size_t i = 0; i < arr.size(); i++) {

				Object obj(arr[i].get_obj());

				string name;
				string val;

				for (size_t j = 0; j < obj.size(); j++) {

					if (obj[j].name_ == "name") {
						name = obj[j].value_.get_str();
					}

					if (obj[j].name_ == "value") {
						val = obj[j].value_.get_str();
					}

				}

				if (tempname == name) {
					return val;
				}

			}

			return string("");

		}

		const bool name_list(vector<string> & names) {

			if (names.size())
				names.clear();

			Value val(get_json("name_list", vector<string>() /* No params */));

			Array arr(val.get_array());

			for (size_t i = 0; i < arr.size(); i++) {

				Object obj(arr[i].get_obj());

				for (size_t j = 0; j < obj.size(); j++) {

					if (obj[j].name_ == "name") {
						names.push_back(obj[j].value_.get_str());
					}

				}
			}
			return true;
		}

		const bool name_scan(const string & name, string & reply, vector<string> & relays) {

			bool found = false;

			try {

				if (relays.size())
					relays.clear();

				vector<string> params;
				params.push_back(name);
				params.push_back("1");

				Value val(get_json("name_scan", params));

				Array arr(val.get_array());

				for (size_t i = 0; i < arr.size(); i++) {
					Object obj(arr[i].get_obj());

					for (size_t j = 0; j < obj.size(); j++) {

						if (obj[j].name_ == "name") {
							if (obj[j].value_.get_str() == name)
								found = true;
						}

						if (obj[j].name_ == "value" && found) {

							std::stringstream entrystream;
							entrystream << obj[j].value_.get_str();

							json_spirit::read_stream(entrystream, val);

							try {

								Object entryval(val.get_obj());

								for (size_t k = 0; k < entryval.size(); ++k) {

									if (entryval[k].name_ == "onion" && entryval[k].value_.type() == 0) {

										Object onionobj(entryval[k].value_.get_obj());

										for (size_t l = 0; l < onionobj.size(); l++) {
											if (onionobj[l].name_ == "reply") {
												reply = onionobj[l].value_.get_str();
											} else if (onionobj[l].name_ == "relays") {

												Array ourrelays(onionobj[l].value_.get_array());

												for (size_t m = 0; m < ourrelays.size(); m++) {
													relays.push_back(ourrelays[m].get_str());
												}
											}
										}
									}
								}
							} catch (const std::exception & ex) {
								throw json_unexpected(ex.what());
							}
						}
					}
				}
			} catch (const char * ex) {
				std::cout << "Exception: " << ex << std::endl;
			}
			return found;
		}

	private:

		json_rpc_caller json;

};

#endif
