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

#include <fstream>
#include <sstream>
#include "get.h"
#include "command.h"
#include "cam2closer.h"

bool isclosercam;

class MySqlValues {

	public:

	MySqlValues(std::string campaign, std::string leadid) {

		std::string mySqlUser, mySqlPass, dbName, mySqlHost;
		std::ifstream mySqlIn;
		mySqlIn.open("astcrm.conf");
		if (!mySqlIn) std::cout << "<b><font color=red>Error! Unable to open config.</font></b>\n";
		std::getline(mySqlIn,mySqlUser,'\n'); // move past the crm passwd
		std::getline(mySqlIn,mySqlUser,'\n');
		std::getline(mySqlIn,mySqlPass,'\n');
		std::getline(mySqlIn,mySqlHost,'\n');
		std::getline(mySqlIn,dbName,'\n');
//		std::getline(mySqlIn,dbName,'\n');
		mySqlIn.close();

//		this allowed the lookup to work see LookupValue below
//		std::string thecommand = "echo \"SELECT * FROM " + campaign + " WHERE id=" + leadid + " || phone=" + leadid + " || name=\\\"" + leadid + "\\\" ORDER BY id ASC LIMIT 1\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName;
		std::string thecommand = "echo \"SELECT * FROM " + campaign + " WHERE id=" + leadid + "\" | mysql -u"  + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName;

		// debug info
		std::cout << "<!-- ";
		std::cout << thecommand;
		std::cout << " -->\n";

//		rawCommand = thecommand;

		std::stringstream MySqlStream;
		MySqlStream << command(thecommand);
		std::getline(MySqlStream,itsRawNames,'\n');
		for (std::string tempLine; std::getline(MySqlStream,tempLine,'\n'); ) {
			itsRawValues += tempLine + "\n";
		}

	}

	~MySqlValues() {}

	bool stateIsListed(const std::string & filename, const std::string & state) {

		bool isListed = false;
		std::ifstream Infile;
		Infile.open(filename.c_str());
		for (std::string tempLine; std::getline(Infile,tempLine,'\n'); ) {
			if (tempLine.find(state,0) != std::string::npos) {
				isListed = true;
			}
		}
		Infile.close();

		return isListed;

	}

	std::string GetValue(const std::string & theField) {

		std::string theValue, theState;
		std::stringstream FieldNameStream, ValueStream;

		if (theField == "nrbflag") {

			theState = GetValue("state");

			if (stateIsListed("norebuttal.txt",theState)) {

				return std::string("<b><font color=red>NO REBUTTALS ALLOWED</font></b>");

			} else {
	
				return std::string("&nbsp;");

			}

		}

		if (theField == "rtpflag") {

			theState = GetValue("state");

			if (stateIsListed("righttoproceed.txt",theState)) {

				return std::string("<b><font color=red>MAY I PROCEED?</font></b>");

			} else {

				return std::string("&nbsp;");

			}

		}

		if (theField == "mpcflag") {

			theState = GetValue("state");

			if (stateIsListed("multipartyconsent.txt",theState)) {

				return std::string("<b><font color=red>MUST DISCLOSE RECORDING</font></b>");

			} else {

				return std::string("&nbsp;");

			}

		}

		if (theField != "nrbflag" && theField != "rtpflag" && theField != "mpcflag") {

			FieldNameStream << itsRawNames;
			ValueStream << itsRawValues;

			for (std::string tempLine; std::getline(FieldNameStream,tempLine,'\t'); ) {

				std::getline(ValueStream,theValue,'\t');
				if (tempLine == theField) {
					return theValue;
				}
			}	

			return std::string("");

		}

		return std::string("");

	}

	private:

	std::string itsRawNames, itsRawValues;

};

void lookupValue(std::string value, std::string campaign, std::string leadid) {

	std::string mySqlUser, mySqlPass, dbName;
	std::ifstream mySqlIn;
	mySqlIn.open("astcrm.conf");
	if (!mySqlIn) std::cout << "<b><font color=red>Error! Unable to open config.</font></b>\n";
	std::getline(mySqlIn,mySqlUser,'\n'); // move past the crm passwd
	std::getline(mySqlIn,mySqlUser,'\n');
	std::getline(mySqlIn,mySqlPass,'\n');
	std::getline(mySqlIn,dbName,'\n');
	std::getline(mySqlIn,dbName,'\n');
	mySqlIn.close();

	std::string thecommand = "echo \"SELECT " + value + " FROM " + campaign + " WHERE id=" +leadid + "\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " " + dbName + " -N";
	std::cout << "<!-- ";
	std::cout << thecommand;
	std::cout << " -->" << std::endl;

	std::cout << command(thecommand);
}

void dumpFile(std::string contents, std::string campaign, std::string leadid) {

	MySqlValues TheMySqlValues(campaign,leadid);
	std::string lastVar;
	bool inVariable = false;
	for (unsigned int i = 0; i < contents.length(); i++) {

	if (contents[i] == '}') {
		inVariable = false;
		//lookupValue(lastVar,campaign,leadid);
		std::cout << TheMySqlValues.GetValue(lastVar);
		lastVar = "";
		i++;
	}
	
	if (contents[i] == '$' && contents[i+1] == '{') {
		inVariable = true;
		i += 2;
	}

	if (inVariable) lastVar += contents[i];

	if (!inVariable) std::cout << contents[i];

	}
}

int main() {

	Get TheFields("text/html");

	std::string campaign = TheFields.GetField("campaign");

	isclosercam = false;

	if (campaign.find("-isclosercam",0) != std::string::npos) {

		campaign = campaign.substr(0,campaign.length()-12);
		isclosercam = true;

	}

	std::string leadid = TheFields.GetField("leadid");
	std::string agent = TheFields.GetField("agent");
	std::string station = TheFields.GetField("station");
	std::string monitor = TheFields.GetField("monitor");

	// MySqlValues TheMySqlValues(campaign,leadid);

	std::string fileContents;
	std::ifstream CampaignFile;
	if (isclosercam) {
		CampaignFile.open(std::string(cam2closer(campaign) + ".script").c_str());
	} else {
		CampaignFile.open(std::string(campaign + ".script").c_str());
	}
	if (!CampaignFile) {
		if (campaign != "0" && campaign != "-1" && campaign != "-2") {
			std::cout << "<b><font color=\"red\">Error! Unable to open script!</font></b>\n";
		}
	}
        
	for (std::string tempLine; std::getline(CampaignFile,tempLine,'\n'); )
		fileContents += tempLine + "\n";

	CampaignFile.close();

	std::cout << "<html><head><title>IRRELEVANT</title>\n";
	std::cout << "<SCRIPT LANGUAGE=\"JavaScript\">\n";
	std::cout << "var cal = new CalendarPopup();\n";
	std::cout << "</SCRIPT>\n";
	std::cout << "</head>\n";

	if (campaign == "0" || campaign == "-1" || campaign == "-2") {
		std::cout << "<body bgcolor=\"#ffffff\">\n";
		std::cout << "<p align=\"right\"><font face=\"helvetica,arial,sans\">";
		// took logout out
		std::cout << "<b><a href=\"/crm/crmlogin.html\" target=\"_parent\"><font color=\"black\"></font></a></b>&nbsp;</p>\n";
	}

	if (campaign != "0" && campaign != "-1" && campaign != "-2") {

	std::cout << "<body bgcolor=\"#ffffff\"";
	if (monitor == "0") {
		std::cout << " onUnLoad=\"document.sendinfo.submit()\"";
	}
  	std::cout << ">\n";

	std::cout << "<p align=\"right\"><font face=\"helvetica,arial,sans\">";

	std::cout << "<b>Campaign:<font color=\"blue\"> " << campaign << " </b>&nbsp;&nbsp;&nbsp;</font>\n";

	std::cout << "<b>Record Id:<font color=\"red\"> " << leadid << " </b>&nbsp;&nbsp;&nbsp;</font>\n";
	std::cout << "<b>Agent:<font color=\"green\"> " << agent << "</b>&nbsp;&nbsp;&nbsp;</font>\n";
	//std::cout << "<b><a href=\"/crm/crmlogin.html\" target=\"_parent\"><font color=\"black\">Logout</font></a></b>&nbsp;</p>\n";

	}

	std::cout << "<form method=\"GET\" name=sendinfo action=\"/cgi-bin/submitinfo.cgi\" target=\"processinfo\">\n";
	// campaign, leadid, agent
	if (isclosercam) {
		std::cout << "<input type=hidden name=campaign value=\"" << campaign << "-isclosercam\">\n";
	} else {
		std::cout << "<input type=hidden name=campaign value=\"" << campaign << "\">\n";
	}
	std::cout << "<input type=hidden name=leadid value=\"" << leadid << "\">\n";
	std::cout << "<input type=hidden name=agent value=\"" << agent << "\">\n";
	std::cout << "<input type=hidden name=monitor value=\"" << monitor << "\">\n";

	if (campaign == "0" || campaign == "-1" || campaign == "-2") {

		std::cout << "<br><br><br><br><center><font face=\"helvetica,arial,sans\">";
		if (campaign == "-2")
			std::cout << "CRM Logged In.";
		if (campaign == "-1")
			std::cout << "Found server... Authenticating...";
		if (campaign == "0")
			std::cout << "Locating server...";
		std::cout << "</font></center>";

  	} else {
  		dumpFile(fileContents,campaign,leadid);
	}

	std::cout << "</form>\n";

	std::cout << "</body></html>\n";

	return 0;

}

