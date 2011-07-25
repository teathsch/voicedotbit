// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

#ifndef MAIN_CPP
#define MAIN_CPP

#include <iostream>
#include <gtkmm.h>
#include <map>
#include <string>
#include "appserver.h"
#include "onionserver.h"
#include "sqlitedb.h"
#include "json.h"
#include "interface.h"
#include "keyvals.h"
#include "resources.h"
//#include "curve25519.h"
#include "base64.h"
#include "parsencconfig.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>
//#include <boost/filesystem.hpp>

using std::cout;
using std::cerr;
using std::endl;

template<typename ResType> bool do_main_loop(ResType & resources) {

	vector<string> names;

	resources.get_json_interface().name_list(names);

	sqlite3_db & db = resources.get_db();

	for (vector<string>::iterator it = names.begin(); it != names.end(); ++it) {

//		cout << "Name to check: " << *it << endl;

		sqlite3_result res;
		db("SELECT `name` FROM `ecdh_keys` WHERE `name`='" + *it + "';", res);

//		cout << res.size() << endl;

		if (res.size() == 0) {

			unsigned char pvt_key[32];
			unsigned char pub_key[32];
			char pvt_key_coded[256];
			char pub_key_coded[256];

//			curve25519_keypairgen(pvt_key, pub_key);
			for (size_t i = 0; i < 32; i++) {
				pvt_key[i] = rand();
				pub_key[i] = rand();
			}

			int len_pvt_key_coded =base64_encode(32,(char *)pvt_key,pvt_key_coded);
			int len_pub_key_coded =base64_encode(32,(char *)pub_key,pub_key_coded);

			cout << "Generating new keypair for " << *it << endl;
			cout << "Pubkey: ";

			for (size_t i = 0; i < len_pvt_key_coded; i++) {
				std::cout << pvt_key_coded[i];
			} std::cout << std::endl << std::endl;

			cout << endl << endl;

			string q = "INSERT INTO `ecdh_keys` VALUES ('" + *it + "', '";

			q.append((char *) pvt_key_coded, len_pvt_key_coded);
			q += "', '";
			q.append((char *) pub_key_coded, len_pub_key_coded);

			q += "');";

			cout << q << endl;

			db(q);
		}
	}
}

template<typename ResType> void main_loop(ResType & resources) {

	for (; true; usleep(10000000)) {

		try {

			do_main_loop<ResType>(resources);

		} catch (const char * ex) {
			cerr << "const char * exception in main loop: " << ex << endl;
		} catch (const std::string & ex) {
			cerr << "std::string exception in main loop: " << ex << endl;
		} catch (const std::exception & ex) {
			cerr << "std::exception in main loop: " << ex.what() << endl;
		} catch (...) {
			cerr << "Unhandled exception in main loop!" << endl;
		}

	}
}

int main(int argc, char ** argv) {

   try {

		cout << "\n\n" << argv[0] << " Copyright 2011 authors.\n";
		cout << "See LICENSE for details.\n" << endl;

		cout << \
		 "This product includes cryptographic software written by Eric Young"
		 "(eay@cryptsoft.com).  This product includes software written by Tim"
		 "Hudson (tjh@cryptsoft.com).\n" << endl;

		cout << "This program is powered by Curve25519.\n" << endl;

		boost::program_options::options_description desc("Allowed Options");

		std::string prog_name = "voicedotbit";

		std::string default_db_file = getenv("HOME") + std::string("/.") + \
		 prog_name + std::string(".sqlite3");

		desc.add_options()
		  ("help,h", "produce help message")
		  (
		   "database-file,d",
		   boost::program_options::value<std::string>()->default_value(
		         default_db_file),
		   "database file"
		  )
		  (
		   "app-port,p",
		    boost::program_options::value<std::string>()->default_value(
		         std::string("9654")),
		   "port to listen for application data"
		  )
		  (
		   "onion-port,o",
		    boost::program_options::value<std::string>()->default_value(
		         std::string("9655")),
		   "port for communicating on the onion network"
		  )
		;

		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		boost::program_options::notify(vm);

		if (vm.count("help")) {

			cerr << desc << endl;

			return 1;
		}

		std::string app_port      = vm["app-port"     ].as<std::string>();
		std::string onion_port    = vm["onion-port"   ].as<std::string>();
		std::string database_file = vm["database-file"].as<std::string>();

		if (atoi(app_port.c_str()) < 1 || atoi(app_port.c_str()) > 65535)
			throw (const char *) "Invalid app port.";

		if (atoi(onion_port.c_str()) < 1 || atoi(onion_port.c_str()) > 65535)
			throw (const char *) "Invalid onion port.";

		std::ifstream test_db_file(database_file.c_str());
		bool firstrun = ! test_db_file;
		test_db_file.close();

		sqlite3_db db(database_file);

		std::deque<std::map<std::string, std::string> > dbres;

		db("SELECT `name` FROM `sqlite_master` "
		        " WHERE `tbl_name` LIKE 'buddies';", dbres);

		if (dbres.size() == 0) {
			db( "CREATE TABLE `buddies` ("
			     " `name`   TEXT    NOT NULL DEFAULT '', "
			     " `alias`  TEXT    NOT NULL DEFAULT '', "
			     " `reply`  TEXT    NOT NULL DEFAULT '');" );
		} else dbres.clear();

		db("SELECT `name` FROM `sqlite_master` "
		        " WHERE `tbl_name` LIKE 'config';", dbres);

		if (dbres.size() == 0) {
			db( "CREATE TABLE `config` ("
			     " `key`   TEXT NOT NULL DEFAULT '', "
			     " `value`  TEXT NOT NULL DEFAULT '');" );
      } else dbres.clear();

		db("SELECT `name` FROM `sqlite_master` "
		        " WHERE `tbl_name` LIKE 'relays';", dbres);

		if (dbres.size() == 0) {
			db( "CREATE TABLE `relays` ("
			     " `address` TEXT NOT NULL DEFAULT '', "
			     " `port`    TEXT NOT NULL DEFAULT '', "
			     " `pubkey`  TEXT NOT NULL DEFAULT '');" );
		} else dbres.clear();

		db("SELECT `name` FROM `sqlite_master` "
		        " WHERE `tbl_name` LIKE 'ecdh_keys';", dbres);

		if (dbres.size() == 0) {
			db( "CREATE TABLE `ecdh_keys` ("
			     " `name`        TEXT NOT NULL DEFAULT '', "
			     " `public_key`  TEXT NOT NULL DEFAULT '', "
			     " `private_key` TEXT NOT NULL DEFAULT '');" );
		} else dbres.clear();

		db("SELECT `name` FROM `sqlite_master` "
		        " WHERE `tbl_name` LIKE 'pending_identities';", dbres);

		if (dbres.size() == 0) {
			db( "CREATE TABLE `pending_identities` ("
			     " `name` TEXT NOT NULL DEFAULT '');" );
		} else dbres.clear();

//		string temp_json_user = defaults(db, "json_user", "me");
//		string temp_json_pass = defaults(db, "json_pass", "password");
//		string temp_json_host = defaults(db, "json_host", "127.0.0.1");
//		string temp_json_port = defaults(db, "json_port", "9332");

		if (firstrun) {

			std::cout << "Looks like the first run... Trying to use values from bitcoin.conf" << std::endl;

			try {

				map<string, string> namecoin_config;
				parse_namecoin_config(namecoin_config);

				// rpcpassword , rpcport , rpcuser
				if (namecoin_config.find("rpcpassword") != namecoin_config.end()) {
					Set(db, "json_pass", namecoin_config["rpcpassword"]);
				}

				if (namecoin_config.find("rpcport") != namecoin_config.end()) {
					Set(db, "json_port", namecoin_config["rpcport"]);
				}

				if (namecoin_config.find("rpcuser") != namecoin_config.end()) {
					Set(db, "json_user", namecoin_config["rpcuser"]);
				}

			} catch (const char * ex) {
				std::cout << "Couldn't find config file. "            << std::endl;
				std::cout << "We'll have to just guess values. "      << std::endl;
				std::cout << "This probably won't work. "             << std::endl;
				std::cout << "You'll have to configure it manually. " << std::endl;
			}

		}

		json_interface json(defaults(db, "json_user", "me"       ),
		                    defaults(db, "json_pass", "password" ),
		                    defaults(db, "json_host", "127.0.0.1"),
		                    defaults(db, "json_port", "9332"     )  );

		defaults(db, "inbound_mode", "2");

		Gtk::Main kit(argc, argv);

		asio::io_service io;

		resources res(json, db);

		boost::thread main_loop_thread(boost::bind(main_loop<resources>, res));

		app_server app_serv(io, atoi(app_port.c_str()));
		onion_server onion_serv(io, atoi(onion_port.c_str()));

		boost::thread io_service_thread(boost::bind(&asio::io_service::run, &io));

		MainWindow_t<resources> window(res);
		Gtk::Main::run(window);

		return 0;

	} catch (const char * ex) {
		cerr << "exception (const char *): " << ex << endl;
	} catch (const std::exception & ex) {
		cerr << "std::exception: " << ex.what() << endl;
	} catch (...) {
		cerr << "Unhandled exception!" << endl;
	}

	return 1;

}

#endif
