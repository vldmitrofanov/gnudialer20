/*
 * astcrm - General purpose CRM
 *
 * General purpose CRM software for asterisk
 *
 * Copyright (C) 2005, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#include <iostream>
#include <string>

class Get {

	public:

	Get() {}
	~Get() {}
	Get(std::string contentType) {
		queryString = "&" + std::string(getenv("QUERY_STRING")) + "&";
		std::cout << "Content-type: " << contentType << "\n" << std::endl;
	}

	std::string GetField(std::string field) {
		return __GetField(field);
	}

	std::string __GetField(std::string field) {
		int fieldstart = queryString.find("&" + field + "=",0);
		int valuestart = queryString.find("=",fieldstart+1) + 1;
		int valueend = queryString.find("&",valuestart);
		return queryString.substr(valuestart,valueend-valuestart);
	}

	private:

	std::string queryString;

};

