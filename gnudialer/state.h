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

#include <iostream>
#include <sstream>
#include <vector>

#define FTCL	8
#define FTCH	21

class State {

public:

State() {

	wdayl = FTCL;
	wdayh = FTCH;
	satl = FTCL;
	sath = FTCH;
	sunl = FTCL;
	sunh = FTCH;
	holl = FTCL;
	holh = FTCH;

	}

~State() {}

void Parse(std::string itsRawText) {

	int pos = 0, end = 0;
	std::stringstream RawStream, CodeStream;
	RawStream << itsRawText;
	for (std::string tempLine; std::getline(RawStream,tempLine,'\n'); ) {

		if (tempLine.length() > 1) {

			if (tempLine[0] == '#') {

				std::getline(RawStream,tempLine,'\n');

				}

			}

		if (tempLine.find("CODES=",0) != std::string::npos) {

			CodeStream << tempLine.substr(6,tempLine.length()-6);
			for (std::string tempCode; std::getline(CodeStream,tempCode,','); ) {
				itsAreaCodes.push_back(tempCode);
				}

			}

		if (tempLine.find("WEEKDAY=",0) != std::string::npos) {

			pos = tempLine.find("=",0);
			end = tempLine.find("-",0);

			wdayl = atoi(tempLine.substr(pos+1,end-pos-1).c_str());

			pos = end + 1;
			end = tempLine.length();

			wdayh = atoi(tempLine.substr(pos,end-pos).c_str());

			}

		if (tempLine.find("SAT=",0) != std::string::npos) {

			pos = tempLine.find("=",0);
			end = tempLine.find("-",0);

			satl = atoi(tempLine.substr(pos+1,end-pos-1).c_str());

			pos = end + 1;
			end = tempLine.length();

			sath = atoi(tempLine.substr(pos,end-pos).c_str());

			}

		if (tempLine.find("SUN=",0) != std::string::npos) {

			pos = tempLine.find("=",0);
			end = tempLine.find("-",0);

			sunl = atoi(tempLine.substr(pos+1,end-pos-1).c_str());

			pos = end + 1;
			end = tempLine.length();

			sunh = atoi(tempLine.substr(pos,end-pos).c_str());

			}

		if (tempLine.find("HOLIDAY=",0) != std::string::npos) {

			pos = tempLine.find("=",0);
			end = tempLine.find("-",0);

			holl = atoi(tempLine.substr(pos+1,end-pos-1).c_str());

			pos = end + 1;
			end = tempLine.length();

			holh = atoi(tempLine.substr(pos,end-pos).c_str());

			}

		if (tempLine.find("ALL=",0) != std::string::npos) {

			pos = tempLine.find("=",0);
			end = tempLine.find("-",0);

			wdayl = atoi(tempLine.substr(pos+1,end-pos-1).c_str());

			pos = end + 1;
			end = tempLine.length();

			wdayh = atoi(tempLine.substr(pos,end-pos).c_str());

			satl = wdayl;
			sunl = wdayl;
			holl = wdayl;

			sath = wdayh;
			sunh = wdayh;
			holh = wdayh;

			}

		}

	}

std::string at(int whichCode) { return itsAreaCodes.at(whichCode); }
int size()const { return itsAreaCodes.size(); }

int GetWdayL()const { return wdayl; }
int GetWdayH()const { return wdayh; }
int GetSatL()const { return satl; }
int GetSatH()const { return sath; }
int GetSunL()const { return sunl; }
int GetSunH()const { return sunh; }
int GetHolL()const { return holl; }
int GetHolH()const { return holh; }

private:

int wdayl, wdayh, satl, sath, sunl, sunh, holl, holh;
std::vector<std::string> itsAreaCodes;

};

State ReturnState(std::string rawState) {

	State TheState;
	TheState.Parse(rawState);
	return TheState;

	}
