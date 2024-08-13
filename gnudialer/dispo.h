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
#include <fstream>
#include "etcinfo.h"


//                -8 == Burned (We give up)      ---o
//                -7 == Disconnected Number         | -- Don't get called back
//                -6 == Fax Machine              ---o
//                -5 == Network Congestion ---o
//                -4 == Busy                  |
//                -3 == Voicemail             | -- Get called back ---o
//                -2 == No Answer             |                       |
//                -1 == Undefined*         ---o                       |
//                                                                    |
//                1 == Not Yet Called** -- Get called back -----------|
//                                                                    |
//                0 == Callback  ---o                                 |
//                2 == No Answer    |                                 |
//                3 == Voicemail    | -- Get called back -------------o
//                4 == Busy         |
//                5 == Fast Busy ---o
//                6 == Fax Machine           ---o
//                7 == Disconnected Number      |
//                8 == Do Not Call              | -- Don't count as contacts
//                9 == Invalid Lead             |
//                10 == No Sale***           ---o
//                11 == No Sale***    ---o These both...
//                12 == Sale          ---o count as contacts

struct dispotype {
        int disponum;
        char *shortname;
        char *longname;
};

struct dispotype dispos[] = {
        { -7, 	"M_Disc", 	"Machine - Disconnect" },
        { -6, 	"M_FaxM", 	"Machine - Fax Machine" },
        { -5, 	"M_Cong", 	"Machine - Congestion" },
        { -4, 	"M_Busy", 	"Machine - Busy" },
        { -3, 	"M_AnsM", 	"Machine - Answering Machine" },
        { -2, 	"M_NoAns", 	"Machine - No Answer" },
        { -1, 	"M_Resv", 	"Machine - Reserved" },
        { 0, 	"Callback", 	"Agent - Callback" },
        { 1, 	"Fresh", 	"Fresh Record" },
        { 2, 	"NoAns", 	"Agent - No Answer" },
        { 3, 	"AnsMach", 	"Agent - Answering Machine" },
        { 4, 	"Busy", 	"Agent - Busy" },
        { 5, 	"FBusy", 	"Agent - Fast Busy" },
        { 6, 	"FaxMach", 	"Agent - Fax Machine" },
        { 7, 	"Disco", 	"Agent - Disconnect" },
        { 8, 	"DNC", 		"Agent - Do Not Call" },
        { 9, 	"Invalid", 	"Agent - Invalid"  },
        { 10, 	"Other", 	"Agent - Other" },
        { 11, 	"NoSale", 	"Agent - No Sale" },
        { 12, 	"Sale", 	"Agent - Sale" },
};

static char *dispo2short(int dispo) {
        unsigned int x;
        for (x=0;x<sizeof(dispos) / sizeof(dispos[0]); x++)
                if (dispos[x].disponum == dispo)
                        return dispos[x].shortname;
        return "Unknown";
}

static char *dispo2long(int dispo) {
        unsigned int x;
        for (x=0;x<sizeof(dispos) / sizeof(dispos[0]); x++)
        	if (dispos[x].disponum == dispo)
                	return dispos[x].longname;
        return "Unknown";
}
                                                                
void __addDispo(const std::string & thedispo, const std::string & filename) {

        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
        OutFile.seekg(-1,std::ios::end);
        OutFile << thedispo << std::endl;
        OutFile.close();

}

void addDispo(const std::string & agent, const std::string & campaign, const std::string & thedispo) {

        __addDispo(thedispo,"/var/log/asterisk/gnudialer/" + agent + "-" + campaign);

}

void __writeDispoFull(const std::string & thedispofull, const std::string & filename) {

        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
        OutFile.seekg(0,std::ios::end);
        OutFile << thedispofull << std::endl;
        OutFile.close();

}

void writeDispoFull(const std::string & agent, const std::string & campaign, const std::string & thedispofull) {

        __writeDispoFull(thedispofull,"/var/log/asterisk/gnudialer/" + agent + "-" + campaign);

}

void __writeDispo(const std::string & thedispo, const std::string & filename) {

        std::fstream OutFile;
        char buf[1];
        
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);
	
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

bool hasDispo(const std::string & agent, const std::string & campaign, const std::string & thedispo) {

        std::fstream OutFile;
        char buf[1];
	std::string filename;
	
	filename = "/var/log/asterisk/gnudialer/" + agent + "-" + campaign;
	
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out);

        //buf = new char [1];
        OutFile.seekp(-2,std::ios::end);
        OutFile.read((char*)buf,1);
        if (*buf != 44) {
                // dispo does exist, return true, else false
                // this function is so we can call back the agent 
                // and get a dispo
		return true;

        }
        OutFile.close();
	return false;

}


void writeDBAbandon(const std::string & agent, const std::string & campaign, const std::string & thedispo, const std::string & leadid) {

        std::string mySqlUser = getMySqlUser();
        std::string mySqlPass = getMySqlPass();
        std::string dbName = getDbName();
        std::string mySqlHost = getMySqlHost();

        std::string theDBQuery = "UPDATE campaign_" + campaign + " SET abandons=abandons+1 WHERE id=" + leadid + "";

        std::system(std::string("echo \"" + theDBQuery + "\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName).c_str());
}        
        
void writeDBDispo(const std::string & agent, const std::string & campaign, const std::string & thedispo, const std::string & leadid) {

        std::string mySqlUser = getMySqlUser();
        std::string mySqlPass = getMySqlPass();
        std::string dbName = getDbName();
        std::string mySqlHost = getMySqlHost();

        std::string theDBQuery = "UPDATE campaign_" + campaign + " SET disposition=" + thedispo + ",agent=" + agent + " WHERE id=" + leadid + "";

        std::system(std::string("echo \"" + theDBQuery + "\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName).c_str());
}

void writeDBString(const std::string & campaign, const std::string & leadid, const std::string & thestring) {

        std::string mySqlUser = getMySqlUser();
        std::string mySqlPass = getMySqlPass();
        std::string dbName = getDbName();
        std::string mySqlHost = getMySqlHost();

        std::string theDBQuery = "UPDATE campaign_" + campaign + " SET " + thestring + " WHERE id=" + leadid + "";

        std::system(std::string("echo \"" + theDBQuery + "\" | mysql -u" + mySqlUser + " -p" + mySqlPass + " -h" + mySqlHost + " " + dbName).c_str());
}


//do not delete this function, it's a needed placeholder
void testWrite(const std::string & agent, const std::string & campaign) {

        std::fstream OutFile;
        OutFile.open(static_cast<std::string>("/var/log/asterisk/gnudialer/300-test301").c_str());
        OutFile.seekg(-1,std::ios::end);
        OutFile << "test" << std::endl;
        OutFile.close();
        //the next 2 lines are here to stop a compile warning
        std::cout << dispo2short(1) << std::endl;
        std::cout << dispo2long(1) << std::endl;
}

//		checkDispo(agent,campaign,thedispo);
//		writeDispo(agent,campaign,thedispo);
//		writeDispo("300","test301","2");
//      	testWrite("300","test301");
// 		writeDBString(campaign,leadid,"disposition='3',agent='300'")
