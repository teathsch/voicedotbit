// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

#ifndef RESOURCES_H
#define RESOURCES_H

// Pass an object around that refers to all of our resources such as
// json and db so that we don't have to pass references for everything

#include "sqlitedb.h"
#include "json.h"
#include "keyvals.h"

template<typename T> class resources_t {

	public:

		resources_t(json_interface & tempjson, sqlite3_db & tempdb) : \
		   current_identity() , \
		   current_buddy()    , \
		   json(tempjson)     , \
		   db(tempdb)         , \
		   inbound_mode(0)    { }

		const int get_inbound_mode() {
			return atoi(Get(db, "inbound_mode").c_str());
		}

		void set_inbound_mode(const int & tempmode) {
			std::stringstream modestream;
			modestream << tempmode;
			Set(db, "inbound_mode", modestream.str());
		}

//		void set_inbound_mode(const int & tempmode) {
//			inbound_mode = tempmode;
//			std::stringstream modestream;
//			modestream << inbound_mode;
//			Set(db, "inbound_mode", );
//		}

		json_interface & get_json_interface()   { return json;             }
		sqlite3_db     & get_db()               { return db;               }
		string         & get_current_identity() { return current_identity; }
		string         & get_current_buddy()    { return current_buddy;    }

	private:

		string current_identity;
		string current_buddy;
		int inbound_mode;
		json_interface & json;
		sqlite3_db     & db;

}; typedef resources_t<int> resources;

#endif
