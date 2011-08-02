// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Wrap sqlite3 calls to create a simple key/value store via the `config` table.

#ifndef KEYVALS_H
#define KEYVALS_H

#include "sqlitedb.h"

using std::string;

template<typename DbType>
DbType & Set(DbType & db, const string & key, const string & value) {

	sqlite3_result res;
	db("SELECT * FROM `config` WHERE `key`='" + escape_string(key) + "';", res);

	if (res.size())
		db("UPDATE `config` SET `value`='" + escape_string(value) +
		       "' WHERE `key`='" + escape_string(key) + "';");
	else
		db("INSERT INTO `config` VALUES ('" + escape_string(key) +
		        "', '" + escape_string(value) + "');");

	return db;
}

template<typename DbType> const string Get(DbType & db, const string & key) {

	sqlite3_result res;
	db("SELECT `value` FROM `config` WHERE `key`='" + escape_string(key) + \
	 "';", res);

	if (!res.size()) {
		return "null";
	}

	return res.front()["value"];

}

template<typename DbType>
string defaults(DbType & db, const string & key, const string & default_value) {

	sqlite3_result res;
	db("SELECT `value` FROM `config` WHERE `key`='" + escape_string(key) +
	       "';", res);

	if (!res.size()) {
		db("INSERT INTO `config` VALUES ('" + escape_string(key) + "', '" +
		      escape_string(default_value) + "');");
		return default_value;
	}

	return res.front()["value"];

}

#endif
