// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Look for .namecoin/bitcoin.conf for namecoin config

#ifndef PARSENCCONFIG_H
#define PARSENCCONFIG_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <map>

using std::string;

template<typename T> const string do_trim(T before) {

	while (before.size() > 0 && before[0] == ' ')
		before.erase(before.find(' '), 1);

	while (before.size() > 0 && before[before.length() - 1] == ' ')
		before.erase(before.rfind(' '), 1);

	return before;

}

template<typename T> void parse_namecoin_config(T & namecoin_config) {

	string dir = "/home/" + string(getenv("USER")) + "/.namecoin/";

	std::ifstream infile(string(dir + "namecoin.conf").c_str());

	if (!infile)
		infile.open(string(dir + "bitcoin.conf").c_str());

	if (!infile)
		throw (const char *) "Can't find file.";

	for (string temp; std::getline(infile, temp, '\n'); ) {

		while(temp.find("\r") != string::npos)
			temp.erase(temp.find("\r"), 1);

		std::stringstream ss;
		ss << temp;

		string name, val;
		std::getline(ss, name, '=' );
		std::getline(ss, val , '\0');

		namecoin_config[do_trim(name)] = do_trim(val);

	}

}

#endif
