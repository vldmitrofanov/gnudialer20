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
#include <vector>
#include <string>
#include <sstream>
#include "etcinfo.h"
#include "Socket.h"
#include "tzpopulate.h"
#include "statepopulate.h"

class Field {

public:

Field() {}
Field(std::string name, std::string value) {
	itsName = name;
	itsValue = value;
	}
~Field() {}

std::string GetName()const { return itsName; }
std::string GetValue()const { return itsValue; }

private:

std::string itsName, itsValue;

};

class FieldList {

public:

FieldList() {}
~FieldList() {}

void AddField(std::string name, std::string value) {

	Field TheField(name,value);
	itsFields.push_back(TheField);

	}

std::string where(std::string name) {

	for (unsigned int i = 0; i < itsFields.size(); i++) {

		if (itsFields.at(i).GetName() == name) {
			return itsFields.at(i).GetValue();
			}

		}

	return "Not Found!";

	}

private:

std::vector<Field> itsFields;

};

std::string stripFeed(std::string & raw) {

	std::string result;
	for (unsigned int i = 0; i < raw.length(); i++) {

		if (raw[i] != '\r') {
			result += raw[i];
			}

		}

	return result;

	}

int main() {

FieldList TheFields;
std::vector<std::string> TheFileLines;

int pos = 0;
std::string queryString;
std::string fileContents;
std::string tempField, tempValue;

bool inFile = false;
for (std::string tempLine; std::getline(std::cin, tempLine, '\n'); ) {

	tempLine = stripFeed(tempLine);

	if (tempLine.find("Content-Disposition: ",0) != std::string::npos && \
	    tempLine.find("filename=",0) == std::string::npos) {

		pos = tempLine.find("=",0) + 2;
		tempField = tempLine.substr(pos,tempLine.length()-pos-1);
		std::getline(std::cin, tempLine, '\n');
		std::getline(std::cin, tempValue, '\n');
		tempValue = tempValue.substr(0,tempValue.length()-1);

		TheFields.AddField(tempField,tempValue);

		}

	if (tempLine.find("Content-Disposition: ",0) != std::string::npos ||
	    tempLine.find("-----------------------------",0) != std::string::npos) {
		inFile = false;
		}

	if (inFile) {

		TheFileLines.push_back(tempLine);

		}

	if (tempLine.find("Content-Type: ",0) != std::string::npos) {
		inFile = true;
		}

	}

//bool inDocument = false;

std::string campaign = TheFields.where("campaign");
std::string queue = campaign;
std::string fieldlist = TheFields.where("fieldlist");
std::string typelist = TheFields.where("typelist");
std::stringstream FieldStream, TypeStream;

FieldStream << fieldlist;
TypeStream << typelist;

std::cout << "Content-type: text/html\n" << std::endl;
std::cout << "<html><body>\n";

// std::cout << "<pre>";

std::cout << "You have loaded the following leads:<br><br>" << std::endl;
std::cout << "Campaign: " << campaign << "<br>" << std::endl;
std::cout << "Fields: " << fieldlist << "<br>" << std::endl;
std::cout << "Types: " << typelist << "<br><br>" << std::endl;
std::cout << "Please check to make sure they were loaded correctly.<br><br>";


std::ofstream CsvOut;
CsvOut.open("tempcsv");
CsvOut << "CREATE TABLE IF NOT EXISTS " << campaign << " (id INT(8) NOT NULL auto_increment ";
for (std::string tempLine, tempLine2; std::getline(FieldStream,tempLine,','); ) {
	std::getline(TypeStream, tempLine2, ',');
        CsvOut << ", ";
	CsvOut << tempLine << " " << tempLine2;
}
CsvOut << ", PRIMARY KEY(id));" << std::endl;
//for (unsigned long int i = 0, j = 1; i < TheFileLines.size(); i++) {
//        if (!TheFileLines.at(i).empty()) {
//                CsvOut << "INSERT INTO " << campaign << " VALUES(" << j << ",";
//                CsvOut << TheFileLines.at(i);
//                CsvOut << ");\n";
//                j++;
//        }
//}

for (unsigned long int i = 0, j = 1; i < TheFileLines.size(); i++) {
	if (!TheFileLines.at(i).empty()) {
		CsvOut << "INSERT INTO " << campaign << "(" + fieldlist + ") VALUES(";
                // old way, just shove whole line in
                // CsvOut << TheFileLines.at(i);
                std::string myTempLine = TheFileLines.at(i);
                while (myTempLine.find(",,",0) != std::string::npos) {
                        int linePos = myTempLine.find(",,",0);
                        myTempLine.insert(linePos+1,"\"\"");
                }
                while (myTempLine.find("\\",0) != std::string::npos) {
                        int linePos = myTempLine.find("\\",0);
                        myTempLine.erase(linePos,1);
                }
                
                CsvOut << myTempLine;
		CsvOut << ");\n";
		j++;
	}
}
CsvOut.close();
std::string theQuery = "mysql -u" + getMySqlUser() + " -p";
theQuery += getMySqlPass() + " -h";
theQuery += getMySqlHost() + " ";
theQuery += getDbName() + " < tempcsv";
std::system(theQuery.c_str());

if (campaign != "DNC") {
	tzpopulate(campaign);
	std::cout << "<br>";
	statepopulate(campaign);
}                        
  std::cout << "<br><form method=post action=\"/cgi-bin/dialeradmin.cgi\">\n";
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
  std::cout << "<input type=submit name=\"mode\" value=\"Main Menu\">";
  std::cout << "</form>";
              
std::cout << "</body></html>\n";

return 0;

}

