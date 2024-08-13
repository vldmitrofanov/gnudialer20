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
#include <fstream>
#include <string>
#include <vector>
#include <signal.h>
#include "cam2closer.h"

bool isclosercam;

// #include "command.h"

void __writeDispo(const std::string & thedispo, const std::string & filename) {

        std::fstream OutFile;
        char buf[1];

        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);

        //buf = new char [1];
        OutFile.seekp(-2,std::ios::end);
        OutFile.read((char*)buf,1);
        if (*buf == 44) {
                // dispo doesn't exist, reset position, and write dispo
                OutFile.seekg(-1,std::ios::end);
                OutFile << thedispo << std::endl;
        }
//        if (*buf == 48) {
//                // dispo is 0 from default crm, reset position, and re-write dispo
//                OutFile.seekg(-2,std::ios::end);
//                OutFile << thedispo << std::endl;
//        }

        OutFile.close();
}

void writeDispo(const std::string & agent, const std::string & campaign, const std::string & thedispo) {

        __writeDispo(thedispo,"/var/log/asterisk/gnudialer/" + agent + "-" + campaign);

}

char hexToDec(char theHexChar) {
	if(theHexChar >= 'A') return (theHexChar & 0xdf) - 'A' + 10;
  	else return theHexChar - '0';
}

std::string dehexify(std::string rawValue) {

	std::string result;
	for (unsigned int i = 0; i < rawValue.length(); i++) {
		if (rawValue[i] == '+') result += " ";
		else if (rawValue[i] != '%') result += rawValue[i];
		else {
			result += hexToDec(rawValue[i + 1]) * 16 + hexToDec(rawValue[i + 2]);
			i += 2;
		}
    	}
	return result;
}

std::string includeQuotes(std::string value) {
	bool foundNonDigit = false;
	for (unsigned int i = 0; i < value.size(); i++)
	if (value[i] < '0' || value[i] > '9')
	if (value[i] != '.')
		foundNonDigit = true;
	if (foundNonDigit || value.empty()) return "\\\"";
	else return "";
}

int main() {

	isclosercam = false;

	std::string rawQuery = "&" + std::string(getenv("QUERY_STRING")) + "&";

	//int pos = 0;

	bool inField = false;
	std::string tempField, tempValue;
	std::vector<std::string> TheFields, TheValues;
	for (unsigned int i = 0; i < rawQuery.length(); i++) {
		if (rawQuery[i] == '&') {
			inField = true;
			if (i > 0) TheValues.push_back(tempValue);
			tempValue = "";
		}
		if (rawQuery[i] == '=') {
			inField = false;
			TheFields.push_back(tempField);
			tempField = "";
		}
		if (inField && rawQuery[i] != '&') {
			tempField += rawQuery[i];
		}
		if (inField == false && rawQuery[i] != '=') {
			tempValue += rawQuery[i];
		}
	}

	std::string campaign = TheValues.at(0);

	if (campaign.find("-isclosercam",0) != std::string::npos) {
		isclosercam = true;
		campaign = campaign.substr(0,campaign.length()-12);
	}

 	std::string leadid = TheValues.at(1);
 	std::string agent = TheValues.at(2);

 	std::string disposition, subdispo;

	std::string mySqlUser, mySqlPass, dbName, mySqlHost;
	std::ifstream mySqlIn;
	mySqlIn.open("astcrm.conf");
	if (!mySqlIn) std::cout << "<b><font color=red>Error! Unable to open config.</font></b>\n";
	std::getline(mySqlIn,mySqlUser,'\n'); // move past the crm passwd
	std::getline(mySqlIn,mySqlUser,'\n');
	std::getline(mySqlIn,mySqlPass,'\n');
	std::getline(mySqlIn,mySqlHost,'\n');
	//  std::getline(mySqlIn,dbName,'\n');
	std::getline(mySqlIn,dbName,'\n');
	mySqlIn.close();

	std::cout << "Content-type: text/html\n" << std::endl;

	std::cout << "<html><head><title>Record Updated</title></head>\n";
	std::cout << "<body bgcolor=\"#ffffff\">\n";

	// std::cout << "Campaign: " << campaign << "<br>\n";
	// std::cout << "Leadid: " << leadid << "<br>\n";
	// std::cout << "Agent: " << agent << "<br>\n";

	std::cout << "Record Successfully Updated!" << std::endl;

	for (unsigned int i = 0; i < TheValues.size(); i++) {
        	TheValues.at(i) = dehexify(TheValues.at(i));
	}

	std::string theQuery;
	if (isclosercam) {
        	theQuery = "echo \"UPDATE campaign_" + campaign + " SET closer='" + agent + "',";
	} else {
        	theQuery = "echo \"UPDATE campaign_" + campaign + " SET agent='" + agent + "',";
	}
        
	for (unsigned int i = 3, j = 0; i < TheFields.size(); i++) {

		if (TheFields.at(i) == "disposition_use") {
			disposition = TheValues.at(i);
		}
		if (TheFields.at(i) == "subdispo_use") {
			subdispo = TheValues.at(i);
			if (isclosercam) {
				TheValues.at(i) = "H" + subdispo;
			//} else {
			//	TheValues.at(i) = "A" + subdispo; 
          		}
		}
            
		if (TheFields.at(i).find("_use",0) != std::string::npos && TheValues.at(i).empty() == false) {
			if (j != 0) theQuery += ",";
			theQuery += " " + TheFields.at(i).substr(0,TheFields.at(i).length()-4) + "=" + includeQuotes(TheValues.at(i)) + TheValues.at(i) + includeQuotes(TheValues.at(i));
			j++;
		}
	}

	theQuery += " WHERE id=" + leadid + "\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName;

	std::cout << "<!-- ";
	std::cout << theQuery;
	//std::cout << "Query hidden, uncomment to debug." << std::endl;
	std::cout << " -->\n";

	std::system(theQuery.c_str());

	std::cout << "</body></html>\n";

	if (isclosercam) {
		writeDispo(agent,cam2closer(campaign),disposition);
	} else {
		writeDispo(agent,campaign,disposition);
	}

	return 0;
}

