// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Wrap sqlite3 calls to create a very easy to use interface.

#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <sqlite3.h>

typedef std::map<std::string, std::string> sqlite3_row;
typedef std::deque<sqlite3_row> sqlite3_result;

template<typename StringType>
const std::string escape_string(StringType temp) {

	std::string res;
	char * buf = sqlite3_mprintf("%q", std::string(temp).c_str());

	if (buf) {
		res.append(buf);
		sqlite3_free(buf);
	}

   return res;
}

template<typename DBType> class sqlite3_db_t {
	public:

		sqlite3_db_t(const std::string & fullpath) : db(NULL) {

			int rc = sqlite3_open(fullpath.c_str(), &db);

			if (rc) {

				char * zErrMsg = 0;
				std::string sErrMsg = zErrMsg;
				sqlite3_free(zErrMsg);
				throw sErrMsg.c_str();
			}

		}

		sqlite3_db_t & operator () (const std::string & q) {
			sqlite3_result res;
			return (*this)(q, res);
		}

		sqlite3_db_t & operator () (const std::string & q, sqlite3_result & res) {
			char * zErrMsg = 0;

			int rc = sqlite3_exec(db, q.c_str(), callback, (void *) &res, &zErrMsg);

			if (rc) {
				std::string sErrMsg = zErrMsg;
				sqlite3_free(zErrMsg);
				throw sErrMsg.c_str();
			}

			return *this;
		}


		static int callback(void * result, int argc, char **argv, char **azColName){

			sqlite3_result & res = *((sqlite3_result *) result);

			sqlite3_row row;

			for (int i = 0; i < argc; i++) {
				row.insert(std::make_pair(azColName[i], argv[i] ? argv[i] : ""));
			}

			res.push_back(row);

			return 0;
		}


		~sqlite3_db_t() { sqlite3_close(db); }

	private:

		sqlite3 * db;

}; typedef sqlite3_db_t<sqlite3 *> sqlite3_db;

#endif
