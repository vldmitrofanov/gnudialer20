/*
 * GnuDialer - Complete, free predictive dialer
 *
 * Complete, free predictive dialer for contact centers.
 *
 * Copyright (C) 2006, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include "state.h"
#include "etcinfo.h"
#include "itos.h"

void statepopulate(std::string campaign) {

	std::string theQuery;
	std::string mySqlUser = getMySqlUser();
	std::string mySqlPass = getMySqlPass();
	std::string dbName = getDbName();
	std::string mySqlHost = getMySqlHost();

	std::string initialQuery = "ALTER TABLE " + campaign;
	initialQuery += " ADD wdayl INT(8)";
	initialQuery += ",ADD wdayh INT(8)";
	initialQuery += ",ADD satl INT(8)";
	initialQuery += ",ADD sath INT(8)";
	initialQuery += ",ADD sunl INT(8)";
	initialQuery += ",ADD sunh INT(8)";
	initialQuery += ",ADD holl INT(8)";
	initialQuery += ",ADD holh INT(8)";

	std::string initializeValuesQuery = "UPDATE " + campaign;
	initializeValuesQuery += " SET wdayl=0";
	initializeValuesQuery += ", wdayh=24";
	initializeValuesQuery += ", satl=0";
	initializeValuesQuery += ", sath=24";
	initializeValuesQuery += ", sunl=0";
	initializeValuesQuery += ", sunh=24";
	initializeValuesQuery += ", holl=0";
	initializeValuesQuery += ", holh=24";

	std::vector<State> TheStates;
	std::vector<std::string> TheQueries;
	TheQueries.push_back(initialQuery);
	TheQueries.push_back(initializeValuesQuery);

	std::ifstream StatesIn;
	StatesIn.open("/usr/lib/gnudialer/timezones/states.txt");
	bool first = true;
	for (std::string tempRawState; std::getline(StatesIn,tempRawState,'['); ) {
		if (!first) {
			TheStates.push_back(ReturnState(tempRawState));
		}
		first = false;
	}
	StatesIn.close();

	std::cout << "Populating state timezone fields..." << std::endl;
        std::cout << "Stage 1 of 2..." << std::endl;
        
	for (unsigned int i = 0; i < TheStates.size(); i++) {
        	std::cout << static_cast<int>(static_cast<float>(i) / static_cast<float>(TheStates.size()) * 100.0) << "\%" << std::endl;

        	theQuery = "UPDATE " + campaign + " SET";
        	theQuery += " wdayl=" + itos(TheStates.at(i).GetWdayL());
        	theQuery += ", wdayh=" + itos(TheStates.at(i).GetWdayH());
        	theQuery += ", satl=" + itos(TheStates.at(i).GetSatL());
        	theQuery += ", sath=" + itos(TheStates.at(i).GetSatH());
        	theQuery += ", sunl=" + itos(TheStates.at(i).GetSunL());
        	theQuery += ", sunh=" + itos(TheStates.at(i).GetSunH());
        	theQuery += ", holl=" + itos(TheStates.at(i).GetHolL());
        	theQuery += ", holh=" + itos(TheStates.at(i).GetHolH());

		theQuery += " WHERE";
		
		for (int j = 0; j < TheStates.at(i).size(); j++) {
			theQuery += " SUBSTRING(phone,1,3)=";
			theQuery += TheStates.at(i).at(j);

			if (j < TheStates.at(i).size() - 1) {
				theQuery += " OR";
			}
		}
		TheQueries.push_back(theQuery);        
	}

        std::cout << "Stage 2 of 2..." << std::endl;
        
	for (unsigned int i = 0; i < TheQueries.size(); i++) {
		std::cout << static_cast<int>(static_cast<float>(i) / static_cast<float>(TheQueries.size()) * 100.0) << "\%" << std::endl;
		std::system(std::string("echo \"" + TheQueries.at(i) + "\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName).c_str());
	}

	std::cout << "Done!" << std::endl;
}
