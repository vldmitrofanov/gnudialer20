/*
 * GnuDialer - Complete, free predictive dialer
 *
 * Complete, free predictive dialer for contact centers.
 *
 * Copyright (C) 2006, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 * Richard Lyman <richard@dynx.net>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "etcinfo.h"

std::string createTzQuery(std::string campaign, \
		          std::string filename, \
		          std::string tzh, std::string tzl) {

	std::string theQuery = "UPDATE campaign_" + campaign + " SET tzl=" + tzl;
	theQuery += ", tzh=" + tzh + " WHERE";
	std::ifstream TzFileIn;
	TzFileIn.open(("/usr/lib/gnudialer/timezones/" + filename).c_str());
	bool first = true;
	for (std::string tempLine; std::getline(TzFileIn,tempLine,'\n'); ) {
		if (tempLine.length() == 3) {

			if (!first) theQuery += " OR";
			first = false;
			theQuery += " SUBSTRING(phone,1,3)=" + tempLine;

		}
	}
	TzFileIn.close();

	return theQuery;
}

void tzpopulate(std::string campaign) {

	std::vector<std::string> TheQueries;

	std::string mySqlUser = getMySqlUser();
	std::string mySqlPass = getMySqlPass();
	std::string dbName = getDbName();
	std::string mySqlHost = getMySqlHost();

	std::string initialQuery = "ALTER TABLE campaign_" + campaign + " ADD tzl INT(8), ADD tzh INT(8), ADD attempts int(8), ADD pickups int(8), ADD abandons int(8), ADD disposition int(8), ADD agent varchar(10), ADD closerdispo int(8), ADD closer varchar(10), ADD subdispo varchar(10) default \'\', ADD lastupdated timestamp NOT NULL default \'1970-01-01 00:00:00\' on update CURRENT_TIMESTAMP, ADD cb_datetime timestamp NOT NULL default \'1970-01-01 00:00:00\', ADD INDEX ( attempts ), ADD INDEX ( pickups ), ADD INDEX ( disposition ), ADD INDEX ( phone )";
	TheQueries.push_back(initialQuery);
	std::string initializeValuesQuery = "UPDATE campaign_" + campaign + " SET tzl=0, tzh=0, attempts=0, pickups=0, abandons=0, disposition=1, agent=\\\"000\\\", closerdispo=0, closer=\\\"000\\\" WHERE disposition is NULL" ;
	TheQueries.push_back(initializeValuesQuery);

	//std::string initializeValuesQuery = "ALTER TABLE campaign_" + campaign + " ADD INDEX ( attempts )" ;
	//TheQueries.push_back(initializeValuesQuery);
	//std::string initializeValuesQuery = "ALTER TABLE campaign_" + campaign + " ADD INDEX ( pickups )" ;
	//TheQueries.push_back(initializeValuesQuery);

	TheQueries.push_back(createTzQuery(campaign,"hawaiian.final","-10","-10"));
	TheQueries.push_back(createTzQuery(campaign,"alaskan.final","-9","-9"));
	TheQueries.push_back(createTzQuery(campaign,"pacific.final","-8","-8"));
	TheQueries.push_back(createTzQuery(campaign,"pacificmountain.final","-7","-8"));
	TheQueries.push_back(createTzQuery(campaign,"mountain.final","-7","-7"));
	TheQueries.push_back(createTzQuery(campaign,"mountaincentral.final","-6","-7"));
	TheQueries.push_back(createTzQuery(campaign,"central.final","-6","-6"));
	TheQueries.push_back(createTzQuery(campaign,"centraleastern.final","-5","-6"));
	TheQueries.push_back(createTzQuery(campaign,"eastern.final","-5","-5"));
	TheQueries.push_back(createTzQuery(campaign,"atlantic.final","-4","-4"));

	std::cout << "Populating main fields/timezone fields..." << std::endl;

	for (unsigned int i = 0; i < TheQueries.size(); i++) {
		std::cout << static_cast<int>(static_cast<float>(i) / static_cast<float>(TheQueries.size()) * 100.0) << "%" << std::endl;

		std::system(std::string("echo \"" + TheQueries.at(i) + "\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName).c_str());

	}

	std::cout << "Done!" << std::endl << std::endl;
}

