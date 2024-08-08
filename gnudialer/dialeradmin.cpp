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
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <mysql.h>
#include <time.h>
#include "post.h"
#include "queue.h"
#include "agent.h"
#include "adminwork.h"
#include "settings.h"
#include "log.h"
#include "dispo.h"
#include "abnhelper.h"
//#include "etcinfo.h"

static char *dispo2short(int dispo);
static char *dispo2long(int dispo);

std::string itos(int i)	{	
	std::stringstream s;	
	s << i;	
	return s.str();
}

long int stoi(const std::string & theString) { 
	return atoi(theString.c_str()); 
}

std::string rfill(std::string theString, int theLength) {        
	std::string s;        
	s += theString + std::string("                                                           ").substr(0,theLength-theString.length());        
	return s.c_str();
}

std::string lfill(std::string theString, int theLength){        
	std::string s;        
	s += std::string("                                                           ").substr(0,theLength-theString.length()) + theString;        
	return s.c_str();
}

void printDateFields(int occurence) {        
	timeval tv;        
	gettimeofday(&tv,NULL);        
	time_t thetime;        
	thetime = tv.tv_sec;        
	tm * ptm = localtime(&thetime);        
	int month = ptm->tm_mon + 1;        
	int day = ptm->tm_mday;        
	int year = ptm->tm_year + 1900;        
	std::cout << "<select name=mon" << occurence << " size=1>\n";        
	for (int i = 1; i < 13; i++) {                
		std::cout << "<option value=" << i;                
		if (i == month) std::cout << " SELECTED";                
		std::cout << ">";                
		if (i == 1) std::cout << "January";                
		if (i == 2) std::cout << "February";                
		if (i == 3) std::cout << "March";                
		if (i == 4) std::cout << "April";                
		if (i == 5) std::cout << "May";                
		if (i == 6) std::cout << "June";                
		if (i == 7) std::cout << "July";                
		if (i == 8) std::cout << "August";                
		if (i == 9) std::cout << "September";                
		if (i == 10) std::cout << "October";                
		if (i == 11) std::cout << "November";                
		if (i == 12) std::cout << "December";                
		std::cout << std::endl;                
	}	
	std::cout << "</select>\n";        
	std::cout << "&nbsp;<select name=day" << occurence << ">\n";        
	for (int i = 1; i < 32; i++) {                
		std::cout << "<option";                
		if (i == day) std::cout << " SELECTED" << std::endl;                
		std::cout << ">" << i << std::endl;                
	}        
	std::cout << "</select>\n";        
	std::cout << "&nbsp;<input type=text name=year" << occurence << " size=4 value=\"" << year << "\">\n";	
}

bool loginError = false;
std::string printAlert(std::string label, std::string text) {  
	return "<a href=\"#\" onClick=\"alert(\'" + text + "\')\">" + label + "</a>\n";  
}

void startTable() {  
	std::cout << "<center><table border=0 cellpadding=10 cellspacing=0><tr><td>\n";  
	std::cout << "<font face=\"helvetica,arial,sans\">\n";  
}

void endCell() {  
	std::cout << "</font></td><td><font face=\"helvetica,arial,sans\">\n";  
}

void endRow() {  
	std::cout << "</font></td></tr><tr><td><font face=\"helvetica,arial,sans\">\n";  
}

void endTable() {  
	std::cout << "</font></td></tr></table></center>\n";  
}

std::string printSubmit(std::string value) {  
	return "<input type=submit name=\"mode\" value=\"" + value + "\">";  
}

std::string printSubmitConfirm(std::string value) {
        return "<input type=submit name=\"mode\" value=\"" + value + "\" onClick=\"return confirm('Are you sure?');\">";
}
        
void printHead(std::string text) {  
	std::cout << "<b><font size=4 face=\"helvetica,arial,sans\">";  
	std::cout << text;  std::cout << "</font></b>";  
}

void printSubHead(std::string text) {  
	std::cout << "<b><font color=\"blue\" face=\"helvetica,arial,sans\">";  
	std::cout << text;  std::cout << "</font></b>";  
}  

void doAdd(Post TheFields) {  
	std::string queue = TheFields.GetField("queue");  
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
	printHead("Added Campaign Member");  
	std::cout << "<br><br>";  
	printSubHead("You have added <br><br>");
	// start debug output
	std::cout << "<!-- debug output\n\n";
	for (int i = 0; i < TheFields.OccurencesOf("agentsnotinqueue"); i++) {	
		std::cout << "~" << TheFields.GetField("agentsnotinqueue",i) << "^\n";	
	}
	std::cout << " -->\n";
	// end debug output	
	AgentList TheAgents;	
	TheAgents.ParseAgentList();	

	std::string agent;          
	for (int i = 0; i < TheFields.OccurencesOf("agentsnotinqueue"); i++) {    
		agent = TheFields.GetField("agentsnotinqueue",i);    
		printSubHead(agent);    
		//    printSubHead(agent + " - " + TheAgents.where(atoi(agent.c_str())).GetName());    
		if (!addQueueMember(atoi(agent.c_str()),queue)) 
			std::cout << "Error adding campaign member!";  
	}    
	
	TheAgents.ParseAgentList();
	        
	printSubHead(" to ");  
	printSubHead(queue);  
	printSubHead(".");            
	std::cout << "<br><br>\n";
}

void doRemove(Post TheFields) {  
	std::string queue = TheFields.GetField("queue");    
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
	printHead("Removed Campaign Member");  
	std::cout << "<br><br>";  
	printSubHead("You removed <br><br>");          
        AgentList TheAgents;
	TheAgents.ParseAgentList();
                
	for (int i = 0; i < TheFields.OccurencesOf("currentagents"); i++) {    
		std::string agent = TheFields.GetField("currentagents",i);    
		printSubHead(agent);    
		if (!removeQueueMember(agent,queue)) 
			std::cout << "Error removing campaign member!";  
	}

        TheAgents.ParseAgentList();
                          
	printSubHead(" from ");  
	printSubHead(queue);  
	printSubHead(".");  
	std::cout << "<br><br>\n";                                
}

void showMenu(Post TheFields) {  
	QueueList TheQueues;  
	TheQueues.ParseQueues();
	std::string queue = TheFields.GetField("queue");
         
	std::cout << "<b><font size=4>GnuDialer Administration</font></b>";  
	std::cout << "<br>\n";  
	startTable();  
	std::cout << "<b><font color=\"blue\">Campaigns:</font></b>";              
	endRow();  
	std::cout << "<select name=queue size=10>\n";  
	for (int i = 0; i < TheQueues.size(); i++) {    
		std::cout << "<option";
		if (queue.empty()) {
			if (i == 0) std::cout << " SELECTED";
		} else {
			if (queue == TheQueues.at(i).GetName()) {
				std::cout << " SELECTED";
			}
		}
		std::cout << " value=\"" << TheQueues.at(i).GetName() << "\">";        
		if (campaignIsRunning(TheQueues.at(i).GetName())) 
			std::cout << "* ";  	
		std::cout << TheQueues.at(i).GetName() + "\n";    
	}  
	std::cout << "</select>\n";  
	std::cout << "<font size=-1 face=\"helvetica,arial,sans\">";  
	std::cout << "<br>* means started";  
	std::cout << "</font>";     
	endCell();
	// put queue buttons here  
	//std::cout << "<button type=\"submit\" name=\"mode\" value=\"Create Campaign\">other lang</button>" << "<br>";
	std::cout << printSubmit("Create Campaign") << "<br>";  
	std::cout << printSubmit("Import Data");
	std::cout << printAlert("[?]","Added the ability to import data to a campaign with existing data.\\nThe field names and types used the second time must match the existing ones.\\nOr be a subset of the first data set.\\nFirst import: phone,name,address,city,state,zip,comments\\nSecond import: phone,name,comments");
	        
        std::cout << "<br><br>";
   
	//  	std::cout << printSubmit("Callist Build") << "<br>";  
	std::cout << printSubmit("Edit Campaign Members") << "<br>";  
	std::cout << printSubmit("Manage Campaign") << "<br>";
	std::cout << printSubmit("Edit Filters") << "<br>";
	        
        std::cout << "<br>";
  
	std::cout << printSubmit("Campaign Stats"); 
	std::cout << printAlert("[?]","Closer numbers provided for reference only, they are not included in Campaign totals!");  
        std::cout << "<br>";

//	std::cout << printSubmit("System Stats"); 
//	std::cout << printAlert("[?]","These are the Machine Stats. (what the dialer did)");  
//        std::cout << "<br>";

//	std::cout << printSubmit("All Campaign Stats"); 
//	std::cout << printAlert("[?]","Closer numbers provided for reference only, they are not included in Campaign totals!");  
//        std::cout << "<br>";
	std::cout << printSubmit("Export Sales");  
	std::cout << printAlert("[?]","The output of this feature is currently hardcoded.\\nYou will need to modify the source code of dialeradmin.cpp\\nto suit your liking.");  
	std::cout << "<br>";          
	endRow();  
	std::cout << "<b><font color=\"blue\">Agents:</font></b>";  
	endRow();  
	AgentList TheAgents;  
	TheAgents.ParseAgentList();  
	std::cout << "<select name=agents MULTIPLE size=10>\n";  
	for (int i = 0; i < TheAgents.size(); i++) {    
		std::cout << "<option value=" << TheAgents.at(i).GetNumber();    
		if (i == 0) std::cout << " SELECTED";    
		std::cout << ">" << TheAgents.at(i).GetNumber() << " - " << TheAgents.at(i).GetName();    
	}  
	std::cout << "</select>\n";  
	endCell();  
	// put agent buttons here  
	std::cout << printSubmit("Create Agent") << "<br>";  
	std::cout << printSubmit("Create Closer") << "<br>"; 
        std::cout << "<br>";

        std::cout << printSubmitConfirm("Delete Agent/Closer") << "<br>";
 
	std::cout << printSubmit("Edit Agent Info");  
	std::cout << printAlert("[?]","This feature only allows you to edit 1 agent/closer at a time.\\nOnly the first highlighted agent is editable.") << "<br>";   
	//  std::cout << printSubmit("Message Agent") << "<br>"; 
 
        std::cout << "<br>";
	std::cout << printSubmit("Agent Stats") << "<br>";  
	std::cout << printSubmit("Agents Times") << "<br>";  
	endTable();  
}

void showEditQueueMembers(Post TheFields) {  
	std::string queuename = TheFields.GetField("queue");  
	std::cout << "<input type=hidden name=\"queue\" value=\"" << queuename << "\">";  
	AgentList TheAgents;  
	TheAgents.ParseAgentList();  
	Queue TheQueue;  
	u_long serverId = 1;
	TheQueue.ParseQueue(queuename, serverId);  
	std::cout << "<font face=\"helvetica,arial,sans\" size=4><b>\n";  
	std::cout << "<br>" + TheQueue.GetName() + "<br><br>\n";  
	std::cout << "</font></b>";  
	std::cout << "<center><table border=0 cellpadding=12 cellspacing=0>\n";  
	std::cout << "<tr>";  
	std::cout << "<td><font face=\"helvetica,arial,sans\" color=\"blue\"><b>\n";  
	std::cout << "Current Members</b></font></td>";  
	std::cout << "<td></td>";  
	std::cout << "<td><font face=\"helvetica,arial,sans\" color=\"blue\"><b>\n";  
	std::cout << "Not in Campaign</b></font></td>";  
	std::cout << "</tr>";  
	std::cout << "<tr>\n  <td>\n";  
	std::cout << "<select name=currentagents MULTIPLE size=10>\n";  
	for (int i = 0; i < TheAgents.size(); i++) {    
		if (TheQueue.HasMemberNumber(TheAgents.at(i).GetNumber())) {      
			std::cout << "<option value=" << TheAgents.at(i).GetNumber();      
			if (i == 0) std::cout << " SELECTED";       
			std::cout << ">";      
			std::cout << TheAgents.at(i).GetNumber() << " - ";      
			std::cout << TheAgents.at(i).GetName() << "<br>\n";      
		}    
	}  
	std::cout << "</select>\n";  
	std::cout << "  </td>\n  <td valign=\"center\"><center>\n";    
	std::cout << "<input type=submit name=\"mode\" value=\"Add\">\n<br><br>";  
	std::cout << "<input type=submit name=\"mode\" value=\"Remove\">\n<br><br>";  
	std::cout << "  </center></td>\n  <td>\n";  
	std::cout << "<select name=agentsnotinqueue MULTIPLE size=10>\n";  
	for (int i = 0; i < TheAgents.size(); i++) {    
		if (!TheQueue.HasMemberNumber(TheAgents.at(i).GetNumber())) {      
			std::cout << "<option value=" << TheAgents.at(i).GetNumber();      
			if (i == 0) std::cout << " SELECTED";      
			std::cout << ">";      
			std::cout << TheAgents.at(i).GetNumber() << " - ";      
			std::cout << TheAgents.at(i).GetName() << "<br>\n";      
		}    
	}  
	std::cout << "</select>\n";  
	std::cout << "  </td>\n</tr></table</center>\n";  
}

void doLogin(Post TheFields) {  
	std::string actualPass = getGnuDialerPassword();  
	if (actualPass != TheFields.GetField("pass")) {    
		std::cout << "Password Incorrect!\n";    
		loginError = true;    
	}  
	if (actualPass == TheFields.GetField("pass")) {    
		showMenu(TheFields);    
	}  
}

void doLogout() {  
	printSubHead("You have logged out.");  
	std::cout << "<br><br>";  
	std::cout << "<a href=\"/gnudialer/dialerlogin.html\">Click here</a> to log back in.";  
}

void doManageCampaign(Post TheFields) {    
	std::string queue = TheFields.GetField("queue");  
	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";    
	addBasicSettings(queue); 	
	bool doStart = campaignIsRunning(queue);  
	if (true) {    
		printHead("Manage Campaign Options");    
		std::cout << "<br><br>";    
		printSubHead("Existing options for ");    
		printSubHead(queue);    
		printSubHead(".");    
		std::cout << "<br><br>";          
		Queue TheQueue;    
		u_long serverId = 1;
		TheQueue.ParseQueue(queue,serverId);    
		std::cout << "<table cols=2>" << std::endl;    
		std::cout << "<tr><td>Dial Mode: 	</td><td>" << TheQueue.GetSetting("function").Get() 	<< "</td></tr>" << std::endl;    
		std::cout << "<tr><td>Callback Today:   </td><td>" << TheQueue.GetSetting("calltoday").Get()	<< "</td></tr>" << std::endl; 
		std::cout << "<tr><td>DNC Scrub:   	</td><td>" << TheQueue.GetSetting("usednc").Get()	<< "</td></tr>" << std::endl; 
		std::cout << "<tr><td>MaxRatio: 	</td><td>" << TheQueue.GetSetting("maxratio").Get() 	<< "</td></tr>" << std::endl;    
		std::cout << "<tr><td>MaxAbandons: 	</td><td>" << TheQueue.GetSetting("maxabandons").Get() 	<< "</td></tr>" << std::endl;    
		std::cout << "<tr><td>MaxLines: 	</td><td>" << TheQueue.GetSetting("maxlines").Get() 	<< "</td></tr>" << std::endl;    
		std::cout << "<tr><td>SleepTime: 	</td><td>" << TheQueue.GetSetting("sleeptime").Get() 	<< "</td></tr>" << std::endl;    
		std::cout << "<tr><td>LowestTime: 	</td><td>" << TheQueue.GetSetting("tzearliest").Get() 	<< "</td></tr>" << std::endl;    
		std::cout << "<tr><td>HighestTime: 	</td><td>" << TheQueue.GetSetting("tzlatest").Get() 	<< "</td></tr>" << std::endl;    
                std::cout << "<tr><td>Timeout:          </td><td>" << TheQueue.GetSetting("timeout").Get()      << "</td></tr>" << std::endl;
		std::cout << "<tr><td>CallerID: 	</td><td>" << TheQueue.GetSetting("callerid").Get() 	<< "</td></tr>" << std::endl;    
		std::cout << "<tr><td>DSP Mode:     	</td><td>" << TheQueue.GetSetting("dspmode").Get()      << "</td></tr>" << std::endl;    
		
                std::string trunk = TheQueue.GetSetting("trunk").Get();
		if (trunk.find("%3A",0) != std::string::npos) {
    			int trunkPos = trunk.find("%3A",0);
    			trunk.erase(trunkPos,3);
    			trunk.insert(trunkPos,":");
   		}
   		
		std::cout << "<tr><td>Trunk:     	</td><td>" << trunk				        << "</td></tr>" << std::endl;    
		std::cout << "<tr><td>Dial Prefix:     	</td><td>" << TheQueue.GetSetting("dialprefix").Get()   << "</td></tr>" << std::endl;    
		std::cout << "<tr><td>Use Closer:   	</td><td>" << TheQueue.GetSetting("usecloser").Get()    << "</td></tr>" << std::endl;    
		std::cout << "<tr><td>Closer Cam:   	</td><td>" << TheQueue.GetSetting("closercam").Get()    << "</td></tr>" << std::endl;    
                std::cout << "<tr><td>Debug:            </td><td>" << TheQueue.GetSetting("debug").Get()        << "</td></tr>" << std::endl;
		
		std::cout << "</table>" << std::endl;        
		std::cout << "<br>" << std::endl;    
		if (doStart) {      
			std::cout << printSubmit("Stop Campaign");    
		} else {      
			std::cout << printSubmit("Start Campaign");      
		}    
		std::cout << printSubmit("Edit Campaign");    
//		std::cout << printSubmit("Edit Filters");      
		std::cout << printSubmit("Edit Script");
//                std::cout << printSubmit("Print Record");
    
		std::cout << "<br>";
		std::cout << printSubmit("Backup Data");
		std::cout << printSubmit("Restore Data");
		std::cout << printSubmit("Create Reload Data");
		std::cout << "<br>";
		std::cout << printSubmit("Database Counts") << "<br>";
		                                                 
		std::cout << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		std::cout << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		std::cout << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		std::cout << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

		std::cout << printSubmitConfirm("Remove Campaign");
		std::cout << "<br>";     
	}  
}

void doPrintRecord(Post TheFields) {
        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";

	printSubHead("Enter the Record Number to Print");
  	std::cout << "<br><br>";

  	std::cout << "<input type=\"text\" name=\"leadid\"><br><br>\n";

        std::cout << "<br><br>";
        std::cout << printSubmit("Print Now");
        std::cout << "<br><br>";
}

void doPrintNow(Post TheFields) {
        std::string queue = TheFields.GetField("queue");
        std::string leadid = TheFields.GetField("leadid");
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
        printHead("Print Record for");
        std::cout << "<br><br>\n";
        printSubHead(queue);
        std::cout << "<br><br>Leadid: ";
        printSubHead(leadid);
        std::cout << "<br><br>\n";
        
	if (!leadid.empty()) {
        	MYSQL *mysql = NULL;
        	MYSQL_RES *result;
        	MYSQL_ROW row;

        	unsigned int counter = 0;

        	mysql = mysql_init(NULL);
        	if(mysql == NULL) {
                	std::cerr << "MySql init failed!" << std::endl;
                }
        	if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
                                getMySqlUser().c_str(), \
                                getMySqlPass().c_str(), \
                                getDbName().c_str(), \
                                3306, NULL, 0)) {
                	std::cerr << "MySql connection failed!" << std::endl;
        	}

	        std::string query = "SELECT * FROM " + queue + " WHERE id=" + leadid + "";


	        if(mysql_query(mysql, query.c_str()) != 0) {
        	        std::cout << "PrintNow: Error selecting record!" << std::endl;
        	} else {

                	result = mysql_use_result(mysql);

                	for(counter = 0; row = mysql_fetch_row(result); counter++) {
                        	writeCampaignSale("Agent SALE",queue,std::string(row[0]));
                	}

	                mysql_free_result(result);
        	}

        	std::cout << "Complete\n";
        	std::cout << "<br><br>";
	} else {
                std::cout << "Leadid was empty!\n";
                std::cout << "<br><br>";
	}
}


void doBackupData(Post TheFields) {
        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
        printHead("Backup Data for");
        std::cout << "<br><br>\n";
        printSubHead(queue);
        std::cout << "<br><br>";
        Queue TheQueue;
		u_long serverId = 1;
        TheQueue.ParseQueue(queue,serverId);
	if (TheQueue.GetSetting("active").Get() == "true") {
		std::cout << "You must stop the campaign first.<br><br>\n";        
	        std::cout << printSubmit("Stop Campaign");
	} else {
                std::cout << "This can take a few minutes!<br><br>\n";
		std::cout << printSubmit("Backup Now");
	}
        std::cout << "<br><br>";
}
                
void doRestoreData(Post TheFields) {
        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
        printHead("Restore Data for");
        std::cout << "<br><br>\n";
        printSubHead(queue);
        std::cout << "<br><br>";
                
        Queue TheQueue;
		u_long serverId = 1;
        TheQueue.ParseQueue(queue,serverId);
	if (TheQueue.GetSetting("active").Get() == "true") {
	        std::cout << "You must stop the campaign first.<br><br>\n";
		std::cout << printSubmit("Stop Campaign");
        } else {
                std::cout << "This can take a few minutes!<br><br>\n";
                std::cout << printSubmit("Restore Now");
        }
        std::cout << "<br><br>";
}

void doBackupNow(Post TheFields) {
        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
        printHead("Backup Data for");
        std::cout << "<br><br>\n";
        printSubHead(queue);

        MYSQL *mysql = NULL;
//        MYSQL_RES *result;
//        MYSQL_ROW row;

//        unsigned int counter = 0;
        std::string query;
        mysql = mysql_init(NULL);
        if (mysql == NULL) {
                std::cerr << "MySql init failed!" << std::endl;
        }
        if (!mysql_real_connect(mysql, getMySqlHost().c_str(), \
                                getMySqlUser().c_str(), \
                                getMySqlPass().c_str(), \
                                getDbName().c_str(), \
                                3306, NULL, 0)) {
                                std::cerr << "MySql connection failed!" << std::endl;
        }

        query = "BACKUP TABLE " + queue +  " TO '/tmp'";

        if(mysql_query(mysql, query.c_str()) != 0) {
                std::cout << "<pre>Error backing up data!</pre>" << std::endl;
        }

        std::cout << "<br><br>";
	std::cout << "Complete\n";
        std::cout << "<br><br>";
        std::cout << printSubmit("Start Campaign");
        std::cout << "<br><br>";
}

void doRestoreNow(Post TheFields) {
        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
        printHead("Restore Data for");
        std::cout << "<br><br>\n";
        printSubHead(queue);

        MYSQL *mysql = NULL;
//        MYSQL_RES *result;
//        MYSQL_ROW row;

//        unsigned int counter = 0;
        std::string query;
        mysql = mysql_init(NULL);
        if (mysql == NULL) {
                std::cerr << "MySql init failed!" << std::endl;
        }
        if (!mysql_real_connect(mysql, getMySqlHost().c_str(), \
                                getMySqlUser().c_str(), \
                                getMySqlPass().c_str(), \
                                getDbName().c_str(), \
                                3306, NULL, 0)) {
                                std::cerr << "MySql connection failed!" << std::endl;
        }


        query = "DROP TABLE " + queue +  "";
        if(mysql_query(mysql, query.c_str()) != 0) {
                std::cout << "<pre>Error dropping table!</pre>" << std::endl;
        }

        query = "RESTORE TABLE " + queue +  " FROM '/tmp'";
        if(mysql_query(mysql, query.c_str()) != 0) {
                std::cout << "<pre>Error restoring data!</pre>" << std::endl;
        }

        std::cout << "<br><br>";
        std::cout << "Complete\n";
        std::cout << "<br><br>";
        std::cout << printSubmit("Start Campaign");
	std::cout << "<br><br>";
}

void doEditScript(Post TheFields) {	
	std::string queue = TheFields.GetField("queue");	
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
	//std::cout << "<input type=hidden value=\"" + queue + "\">\n";	
	printHead("Edit Script");	
	std::cout << "<br><br>\n";	
	printSubHead(queue);	
	std::cout << "<br><br>";	
	std::ifstream InFile;	
	InFile.open((queue + ".script").c_str());	
	std::cout << "<textarea name=thescript rows=20 cols=80>";	
	for (std::string tempLine; std::getline(InFile,tempLine,'\n'); ) {		
		std::cout << tempLine << std::endl;		
	}	
	std::cout << "</textarea>";	
	std::cout << "<br><br>";	
	std::cout << printSubmit("Save Script");	
	std::cout << "<br><br>";	
	InFile.close();	
}

void doSaveScript(Post TheFields) {	
	std::string queue = TheFields.GetField("queue");	
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
	printHead("Saving Script...");	
	std::cout << "<br><br>\n";	
	printSubHead(queue + " saved.");	
	std::cout << "<br><br>\n";	
	std::cout << "<pre>";	
	std::cout << TheFields.GetField("thescript");	
	std::cout << "</pre>";
		std::ofstream OutFile;
		OutFile.open((queue + ".script").c_str());
		OutFile << TheFields.GetField("thescript");
		OutFile.close();	
}

const std::string selectedIf(const std::string & lhs, const std::string & rhs) {	
	if (lhs == rhs) return "SELECTED";	
	else return "";	
}

void doEditGlobals(Post TheFields) {      	
	std::string queue = TheFields.GetField("queue");	            	
	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";        
	Queue TheQueueGlobals;    
	u_long serverId = 1;    
	TheQueueGlobals.ParseQueue("general", serverId);	
	std::string debug	= TheQueueGlobals.GetSetting("debug").Get();    	
	std::string log		= TheQueueGlobals.GetSetting("log").Get();    	
        
	printHead("Edit Globals");    	
	std::cout << "<br><br>";    	
	printSubHead("Enter GnuDialer Global options");    	
	std::cout << "<br><br>";    	
	std::cout << "<div align=left><table cols=2>" << std::endl;        
	std::cout << "<tr><td>";        

	std::cout << "Debug: ";        
	std::cout << printAlert("[?]","Send GnuDialer debug output to the console.");        
	std::cout << "</td><td>";        
	std::cout << "<select name=\"debug\" size=1>";        
	std::cout << "<option value=true " << selectedIf("true",debug) << ">Yes\n";        
	std::cout << "<option value=false " << selectedIf("false",debug) << ">No\n";        
	std::cout << "</select>";        
	std::cout << "</td></tr>";        
	std::cout << "<tr><td>";        

	std::cout << "Log: ";        
	std::cout << printAlert("[?]","Send GnuDialer log output to gnudialer.log.");        
	std::cout << "</td><td>";        
	std::cout << "<select name=\"log\" size=1>";        
	std::cout << "<option value=true " << selectedIf("true",log) << ">Yes\n";        
	std::cout << "<option value=false " << selectedIf("false",log) << ">No\n";        
	std::cout << "</select>";        
	std::cout << "</td></tr>";

	std::cout << "</table></div>";        
	std::cout << "<br><br>";    	
	std::cout << printSubmit("Save Global Settings");	
	std::cout << "<font size=-1 face=\"helvetica,arial,sans\">";	
	std::cout << "<br>(changes take effect the next Update cycle (5-20 secs))";	
	std::cout << "</font>";     	
	std::cout << "<br><br>";  
}

void doFinishEditingGlobals(Post TheFields) {    	
	std::string queue = TheFields.GetField("queue");        
	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";        	
	std::string debug	= TheFields.GetField("debug");	
	std::string log		= TheFields.GetField("log");         
	Queue TheQueueGlobals;  
	u_long serverId = 1;      
	TheQueueGlobals.ParseQueue("general",serverId);        
	TheQueueGlobals.SetSetting("debug",	debug);        
	TheQueueGlobals.SetSetting("log",	log);
	TheQueueGlobals.Write();        
	printHead("Save Globals");        
	std::cout << "<br><br>\n";        
	printSubHead("Global settings updated");        
	std::cout << "<br><br>";  
}

void doAddFilter(Post TheFields) {        
	std::string queue = TheFields.GetField("queue");        
	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";        
	std::string filter       = TheFields.GetField("filter");        
	if (!filter.empty()) {          	
		Queue TheQueue;  
		u_long serverId = 1;      	
		TheQueue.ParseQueue(queue,serverId);        	
		int f;        	
		for (int i = 0; i < TheQueue.OccurencesOf("filters"); i++) {               		
			f = i+1;                     		
		}               		
		if (!TheQueue.SettingExists("filters","number",itos(f))) {               		
			TheQueue.AddSetting("filters","number:" + itos(f) + ":enable:false:string:" + filter);               		
		}                                                       		
		TheQueue.Write();	        
		printHead("Save Filter");	        
		std::cout << "<br><br>\n";	        
		printSubHead("Filter added");	        
		std::cout << "<br><br>filter" + itos(f) + ":" + filter;	        
		std::cout << "<br><br>";	
	} else {		
		printHead("Save Filter");		
		std::cout << "<br><br>\n";		
		printSubHead("Filter was empty, try again.");		
		std::cout << "<br><br>";	                                                                                	
	}  
}
	
void doDelFilter(Post TheFields) {        
	std::string queue = TheFields.GetField("queue");        
	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";        
	std::string fnum         = TheFields.GetField("fnum");	
	std::string filter 	 = TheFields.GetField("filter" + fnum);                
	if (!fnum.empty()) {                	
		Queue TheQueue;    
		u_long serverId = 1;    	
		TheQueue.ParseQueue(queue, serverId);                	
		for (int i = 0; i < TheQueue.OccurencesOf("filters"); i++) {	        	
			if (TheQueue.SettingExists("filters","number",fnum)) {        	        	
				TheQueue.DelSetting("filters","number",fnum);        	        	
			}			
		}	        
		TheQueue.Write();	        
		printHead("Deleted Filter");	        
		std::cout << "<br><br>\n";	        
		printSubHead("Filter deleted");	        
		std::cout << "<br><br>filter" + fnum + ":" + filter;	        
		std::cout << "<br><br>";	
	} else {                
		printHead("Deleted Filter");                
		std::cout << "<br><br>\n";                
		printSubHead("Filter not selected, nothing deleted!");                
		std::cout << "<br><br>";		
	}  
}

void doAreacodeFilter(std::string theCampaign, std::string theSettingName, std::string theSettingValue) {	
	MYSQL *mysql = NULL;	
	MYSQL_RES *result;	
	MYSQL_ROW row;		
	unsigned int counter = 0;

	mysql = mysql_init(NULL);
	if(mysql == NULL) {
       		std::cerr << "MySql init failed!" << std::endl;
      	}
        	
	if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
				getMySqlUser().c_str(), \
				getMySqlPass().c_str(), \
				getDbName().c_str(), \
				3306, NULL, 0)) {
        	std::cerr << "MySql connection failed!" << std::endl;
	}
        	
        std::string query = "SELECT DISTINCT SQL_CALC_FOUND_ROWS left( phone, 3 ) FROM " + theCampaign + "";
        if(mysql_query(mysql, query.c_str()) != 0) {
                std::cout << "<pre>Error selecting areacodes!</pre>" << std::endl;
	} else {
	        result = mysql_use_result(mysql);
        
        	std::cout << "<select name=" + theSettingName + " size=1 value=\"" + theSettingValue + "\">\n";
        	if (theSettingValue == "0") {
			 std::cout << "<option value=\"0\"SELECTED>None<br>\n";
		} else { 
			std::cout << "<option value=\"0\">None<br>\n"; 
		}
        	for(counter = 0; row = mysql_fetch_row(result); counter++) {
        		if (theSettingValue == std::string(row[0])) {
				 std::cout << "<option SELECTED>" << std::string(row[0]) << "<br>\n";
               		} else { 
				std::cout << "<option value=" << std::string(row[0]) << ">" << std::string(row[0]) << "<br>\n"; 
			}
		}
        	std::cout << "</select>\n<br>";
        	mysql_free_result(result);
	}
}
	
void doAreacodePrefixFilter(std::string theCampaign, std::string theSettingName, std::string theSettingValue) {        
	MYSQL *mysql = NULL;        
	MYSQL_RES *result;        
	MYSQL_ROW row;                
	unsigned int counter = 0;                
	mysql = mysql_init(NULL);        
	if(mysql == NULL) {                
		std::cerr << "MySql init failed!" << std::endl;                
	}        
	if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
					getMySqlUser().c_str(), \
					getMySqlPass().c_str(), \
					getDbName().c_str(), \
					3306, NULL, 0)) {                
		std::cerr << "MySql connection failed!" << std::endl;                
	} 

	std::string query = "SELECT DISTINCT SQL_CALC_FOUND_ROWS left( phone, 6 ) FROM " + theCampaign + "";                
	if(mysql_query(mysql, query.c_str()) != 0) {                
		std::cout << "<pre>Error selecting areacodes!</pre>" << std::endl;                
	} else {                       
		result = mysql_use_result(mysql);                
		std::cout << "<select name=" + theSettingName + " size=1 value=\"" + theSettingValue + "\">\n";        
		if (theSettingValue == "0") {
			std::cout << "<option value=\"0\"SELECTED>None<br>\n";                
		} else { 
			std::cout << "<option value=\"0\">None<br>\n"; 
		}        
		for(counter = 0; row = mysql_fetch_row(result); counter++) {                
			if (theSettingValue == std::string(row[0])) {
				std::cout << "<option SELECTED>" << std::string(row[0]) << "<br>\n";                        
			} else { 
				std::cout << "<option value=" << std::string(row[0]) << ">" << std::string(row[0]) << "<br>\n"; 
			}                
		}        
		std::cout << "</select>\n<br>";        
		mysql_free_result(result);        
	}
}

void doZipcodeFilter(std::string theCampaign, std::string theSettingName, std::string theSettingValue) {
        MYSQL *mysql = NULL;
        MYSQL_RES *result;
        MYSQL_ROW row;
        
        unsigned int counter = 0;
        
        mysql = mysql_init(NULL);
        if(mysql == NULL) {
                std::cerr << "MySql init failed!" << std::endl;
	}
        if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
        			getMySqlUser().c_str(), \
        			getMySqlPass().c_str(), \
        			getDbName().c_str(), \
                                3306, NULL, 0)) {
                std::cerr << "MySql connection failed!" << std::endl;
	}
                
        std::string query = "SELECT DISTINCT SQL_CALC_FOUND_ROWS left( zip, 5 ) FROM " + theCampaign + "";
        if(mysql_query(mysql, query.c_str()) != 0) {
                std::cout << "<pre>Error selecting zipcodes!</pre>" << std::endl;
	} else {
	        result = mysql_use_result(mysql);
        
        	std::cout << "<select name=" + theSettingName + " size=1 value=\"" + theSettingValue + "\">\n";
        	if (theSettingValue == "0") {
			 std::cout << "<option value=\"0\"SELECTED>None<br>\n";
                } else { 
			std::cout << "<option value=\"0\">None<br>\n"; 
		}
                
	        for(counter = 0; row = mysql_fetch_row(result); counter++) {
        		if (theSettingValue == std::string(row[0])) {
				 std::cout << "<option SELECTED>" << std::string(row[0]) << "<br>\n";
               		} else { 
				std::cout << "<option value=" << std::string(row[0]) << ">" << std::string(row[0]) << "<br>\n"; 
			}
		}
        	std::cout << "</select>\n<br>";
        	mysql_free_result(result);
	}
}


void doEditFilters(Post TheFields) {
  
    	std::string queue 		= TheFields.GetField("queue");
    	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";

    	Queue TheQueue;
		u_long serverId = 1;
    	TheQueue.ParseQueue(queue,serverId);

    	std::string filter            = TheQueue.GetSetting("filter").Get();
    	std::string usetzfilter       = TheQueue.GetSetting("usetzfilter").Get();
    	std::string skip              = TheQueue.GetSetting("skip").Get();
    	std::string f_mode            = TheQueue.GetSetting("f_mode").Get();
    	std::string f_areacode        = TheQueue.GetSetting("f_areacode").Get();
    	std::string f_areacode_prefix = TheQueue.GetSetting("f_areacode_prefix").Get();
    	std::string f_zipcode         = TheQueue.GetSetting("f_zipcode").Get();
        std::string orderby           = TheQueue.GetSetting("orderby").Get();
        
    	if (f_mode == "0") f_mode = "none";
    
    	if (filter == "0") filter = "";
    
    	MYSQL *mysql = NULL;
    	MYSQL_RES *result;
    	MYSQL_ROW row;
    
  //  	unsigned int counter = 0;
    	std::string query;
    	mysql = mysql_init(NULL);
    	if (mysql == NULL) {
    		std::cerr << "MySql init failed!" << std::endl;
	}
	if (!mysql_real_connect(mysql, getMySqlHost().c_str(), \
    				getMySqlUser().c_str(), \
    				getMySqlPass().c_str(), \
    				getDbName().c_str(), \
    				3306, NULL, 0)) {
    				std::cerr << "MySql connection failed!" << std::endl;
    	}

    	printHead("Edit Filters");
    	std::cout << "<br><br>";
    	printSubHead("Enter filter options for ");
    	printSubHead(queue);
    	printSubHead(".");
    	std::cout << "<br><br>";
  
    	std::cout << "<div align=left><table cols=2>" << std::endl;


        std::cout << "<tr><td>";
        std::cout << "Filter Mode: ";
        std::cout << printAlert("[?]","Allow the ability to enable Areacode, Areacode/Prefix, Zipcode filtering.\\n(changing between types will zero-out previously selected types)\\n");
        std::cout << "</td><td>";
        std::cout << "<select name=\"f_mode\" size=1>";
	if (f_mode == "none") std::cout << "<option value=none SELECTED>None<br>\n";
      		else { std::cout << "<option value=none>None<br>\n"; }
        if (f_mode == "areacode") std::cout << "<option value=areacode SELECTED>Areacode<br>\n";
                else { std::cout << "<option value=areacode>Areacode<br>\n"; }
        if (f_mode == "acprefix") std::cout << "<option value=acprefix SELECTED>AC/Prefix<br>\n";
                else { std::cout << "<option value=acprefix>AC/Prefix<br>\n"; }
        if (f_mode == "zipcode") std::cout << "<option value=zipcode SELECTED>Zipcode<br>\n";
                else { std::cout << "<option value=zipcode>Zipcode<br>\n"; }
        std::cout << "</select>";
        std::cout << "</td></tr>";

	if (f_mode == "areacode") {                         
        	std::cout << "<tr><td>Areacode Filter: ";
		std::cout << printAlert("[?]","Filter based on available areacodes in the campaign database.\\n(select None, if you do NOT want to filter by areacode)");
		std::cout << "</td><td>";
        	doAreacodeFilter(queue,"f_areacode",f_areacode);
		std::cout << "</td></tr>";
        	}

	if (f_mode == "acprefix") {
        	std::cout << "<tr><td>Areacode/Prefix Filter: ";
		std::cout << printAlert("[?]","Filter based on available areacode/prefix conbinations in the campaign database.\\n(select None, if you do NOT want to filter by areacode/prefix)");
        	std::cout << "</td><td>";
		doAreacodePrefixFilter(queue,"f_areacode_prefix",f_areacode_prefix);
		std::cout << "</td></tr>";
		}

	if (f_mode == "zipcode") {
        	std::cout << "<tr><td>Zipcode Filter: ";
        	std::cout << printAlert("[?]","Filter based on available zipcodes in the campaign database.\\n(select None, if you do NOT want to filter by areacode/prefix)");
        	std::cout << "</td><td>";
        	doZipcodeFilter(queue,"f_zipcode",f_zipcode);
        	std::cout << "</td></tr>";
		}

        std::cout << "<tr><td>";        
	std::cout << "Extra Filter: ";
	std::cout << printAlert("[?]","These are AND separated MySQL conditionals to limit lead selection via specified criteria.\\n\\n\\n EXAMPLE: left(phone,3)=\\'408\\'\\nEXAMPLE: left(phone,3)=\\'408\\' AND MID(phone,4,3)=\\'376\\' \\nEXAMPLE: UCASE(city)=\\'SAN JOSE\\' AND UCASE(state)=\\'CA\\' \\nThen click Save Campaign Settings. \\n(if you click Add Filter it will add it to primary filters)");
        std::cout << "</td><td>";
	std::cout << "<input type=text name=\"filter\" size=40 value=\"" + filter + "\">";
	std::cout << "</td></tr>";	        

        std::cout << "<tr><td></td><td>";
        std::cout << printSubmit("Add Filter");
        std::cout << " (to primary filters) ";
        std::cout << printAlert("[?]","Fill in Extra Filter with the query string, then click Add Filter.\\n(this will add it to primary filters and not to the Extra Filter for the campaign)");
        std::cout << "</td></tr>";

                        
        std::cout << "<tr><td  colspan=2>";
	std::string fnum,fstring,enabled;
	std::cout << "<font size=-1 color=\"blue\" face=\"helvetica,arial,sans\">Primary Filters</font><br>&nbsp;";
        std::cout << printAlert("[?]","This check box allows you to enable a filter. \\nThen click Save Campaign Settings.\\nYou must check at least 1 box, or it will default to filter0.");
        std::cout << "<font size=-1 color=\"green\" face=\"helvetica,arial,sans\">";
        std::cout << "Enable</font>&nbsp;";
        
        std::cout << printAlert("[?]","This check box allows you to delete a filter. \\nCheck a delete box, then click Del Filter.\\n\\nYou can NOT delete an Enabled filter, disable it first.\\n Then click Save Campaign Settings.\\nReturn to this menu, you will be able to check the Delete box for that filter. \\nThen click Del Filter.");        
        std::cout << "<font size=-1 color=\"red\" face=\"helvetica,arial,sans\">";
	std::cout << "Delete</font>&nbsp;";
	
        std::cout << printAlert("[?]","This is the realtime record count for this filter.");        
	std::cout << "<font size=-1 color=\"blue\" face =\"helvetica,arial,sans\">";
	std::cout << "Counts</font>&nbsp;";
	std::cout << "<br>";
                
	for (int i = 0; i < TheQueue.OccurencesOf("filters"); i++) {
		fnum = TheQueue.GetSetting(i,"filters").GetAttribute("number");
                fstring = TheQueue.GetSetting(i,"filters").GetAttribute("string");
		enabled = TheQueue.GetSetting(i,"filters").GetAttribute("enable");	

		query = "SELECT COUNT(*) FROM " + queue + " WHERE left(lastupdated,10) <> left(now(),10) AND " + fstring + "";
		if (mysql_query(mysql, query.c_str()) != 0) {
			std::cout << "<pre>Error selecting filter!</pre>" << std::endl;
		} else {
			result = mysql_use_result(mysql);
			row = mysql_fetch_row(result);

			if (enabled == "true") {
        			std::cout << "<input type=checkbox checked name=\"filter" + fnum + "\" value=\"" + fstring + "\"> filter" + fnum + ": " + "<font size=-1 color=\"blue\" face=\"helvetica,arial,sans\">[" + row[0] + "]</font>&nbsp;" + fstring + "\n<br>";
			} else {
				std::cout << "<input type=checkbox name=\"filter" + fnum + "\" value=\"" + fstring + "\"> filter" + fnum + ":&nbsp;<input type=checkbox name=\"fnum\" value=\"" + fnum + "\">" + "<font size=-1 color=\"blue\" face=\"helvetica,arial,sans\">[" + row[0] + "]</font>&nbsp;" + fstring + "\n<br>";
			}
			mysql_free_result(result);
		}
	}       
        std::cout << "</td></tr>";
        std::cout << "<tr><td></td><td>";
        std::cout << printSubmit("Del Filter");
	std::cout << " (from primary filters) ";
        std::cout << printAlert("[?]","Must check 1 delete box, then click Del Filter.");
	std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Use tzFilter: ";
        std::cout << printAlert("[?]","Allow the ability to disable tzfilter (Time Zone Filtering).");
        std::cout << "</td><td>";
        std::cout << "<select name=\"usetzfilter\" size=1>";
        std::cout << "<option value=true " << selectedIf("true",usetzfilter) << ">Yes\n";
        std::cout << "<option value=false " << selectedIf("false",usetzfilter) << ">No\n";
        std::cout << "</select>";
        std::cout << "</td></tr>";
                 
	std::cout << "<tr><td>";
	std::cout << "Skip: ";
	std::cout << printAlert("[?]","This is the number of records that will be skipped over in the current query.\\nThis should be reset to 0 when you change filters.");
	std::cout << "</td><td>";
	std::cout << "<input type=text name=\"skip\" size=7  value=\"" + skip + "\">";
	std::cout << "</td></tr>";
        
        std::cout << "<tr><td>";
        std::cout << "Order By: ";
        std::cout << printAlert("[?]","Allow the ability to order the calling data by, attempts plus pickups (default)\\nby phone and by id (record order)\\n");
        std::cout << "</td><td>";
        std::cout << "<select name=\"orderby\" size=1>";
	if (orderby == "attempts_pickups") std::cout << "<option value=attempts_pickups SELECTED>default<br>\n";
      		else { std::cout << "<option value=attempts_pickups>default<br>\n"; }
        if (orderby == "phone") std::cout << "<option value=phone SELECTED>phone<br>\n";
                else { std::cout << "<option value=phone>phone<br>\n"; }
        if (orderby == "id") std::cout << "<option value=id SELECTED>id<br>\n";
                else { std::cout << "<option value=id>id<br>\n"; }
        std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "</table></div>";
        std::cout << "<br>";
            
	std::cout << printSubmit("Save Filter Settings");
 
        std::cout << "<font size=-1 face=\"helvetica,arial,sans\">";
        std::cout << "<br>(changes take effect the next Update cycle (5-20 secs))";
        std::cout << "</font>";
        
    std::cout << "<br>";
  }

void doFinishEditingFilterSettings(Post TheFields) {

  std::string queue             = TheFields.GetField("queue");
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
  std::string filter            = TheFields.GetField("filter");
  std::string usetzfilter       = TheFields.GetField("usetzfilter");
  std::string skip              = TheFields.GetField("skip");
  std::string f_mode            = TheFields.GetField("f_mode");
  std::string f_areacode        = TheFields.GetField("f_areacode");
  std::string f_areacode_prefix = TheFields.GetField("f_areacode_prefix");
  std::string f_zipcode         = TheFields.GetField("f_zipcode");
  std::string orderby           = TheFields.GetField("orderby");
    
  Queue TheQueue;
  u_long serverId = 1;
  TheQueue.ParseQueue(queue,serverId);
        int f = 0;
  	for (int i = 0; i < TheQueue.OccurencesOf("filters"); i++) {
    		TheQueue.GetSetting(i,"filters").SupAttribute("enable","false");
    		std::string filters = TheFields.GetField("filter" + itos(i));
    		if (!filters.empty()) {
  			TheQueue.GetSetting(i,"filters").SupAttribute("enable","true");
  			f++;
		}  		
	}
  if (!f) TheQueue.GetSetting(0,"filters").SupAttribute("enable","true");        
  if (filter.empty()) filter = "0";
  if (filter == "None") filter = "0";
  if (filter == "none") filter = "0";
  if (skip.empty()) skip = "0";
  if (f_mode == "none") { 
	f_mode = "0";
	f_areacode = "0";
	f_areacode_prefix = "0";
	f_zipcode = "0";  
  }
  if (f_areacode.empty()) f_areacode = "0";
  if (f_areacode_prefix.empty()) f_areacode_prefix = "0";
  if (f_zipcode.empty()) f_zipcode = "0";
  if (f_areacode == "selected") f_areacode = "0";
  if (f_areacode_prefix == "selected") f_areacode_prefix = "0";
  if (f_zipcode == "selected") f_zipcode = "0";
 
  TheQueue.SetSetting("filter",         	filter);
  TheQueue.SetSetting("usetzfilter",    	usetzfilter);
  TheQueue.SetSetting("skip",           	skip);
  TheQueue.SetSetting("f_mode",                 f_mode);
  TheQueue.SetSetting("f_areacode",     	f_areacode);
  TheQueue.SetSetting("f_areacode_prefix", 	f_areacode_prefix);
  TheQueue.SetSetting("f_zipcode",      	f_zipcode);
  TheQueue.SetSetting("orderby",                orderby);
  
  TheQueue.Write();

  printHead("Save Filter Settings");
  std::cout << "<br><br>\n";
  printSubHead("Filters updated for " + queue);

  std::cout << "<br><br>";
  }                                  

//working here
void doShowDBCounts(Post TheFields) {
  
    	std::string queue 		= TheFields.GetField("queue");
    	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";

    	Queue TheQueue;
		u_long serverId = 1;
    	TheQueue.ParseQueue(queue,serverId);

    	std::string filter            = TheQueue.GetSetting("filter").Get();
    	std::string usetzfilter       = TheQueue.GetSetting("usetzfilter").Get();
    	std::string skip              = TheQueue.GetSetting("skip").Get();
    	std::string f_mode            = TheQueue.GetSetting("f_mode").Get();
    	std::string f_areacode        = TheQueue.GetSetting("f_areacode").Get();
    	std::string f_areacode_prefix = TheQueue.GetSetting("f_areacode_prefix").Get();
    	std::string f_zipcode         = TheQueue.GetSetting("f_zipcode").Get();
        
    	if (f_mode == "0") f_mode = "none";
    
    	if (filter == "0") filter = "";
    
    	MYSQL *mysql = NULL;
    	MYSQL_RES *result;
    	MYSQL_ROW row;
    
  //  	unsigned int counter = 0;
    	std::string query;
    	mysql = mysql_init(NULL);
    	if (mysql == NULL) {
    		std::cerr << "MySql init failed!" << std::endl;
	}
	if (!mysql_real_connect(mysql, getMySqlHost().c_str(), \
    				getMySqlUser().c_str(), \
    				getMySqlPass().c_str(), \
    				getDbName().c_str(), \
    				3306, NULL, 0)) {
    				std::cerr << "MySql connection failed!" << std::endl;
    	}

    	printHead("Campaign Counts");
    	std::cout << "<br><br>";
    	printSubHead("For Campaign: ");
    	printSubHead(queue);
    	std::cout << "<br><br>";
  
    	std::cout << "<div align=left><table cols=2>" << std::endl;
	std::cout << "<tr><td>Disposition</td><td>Counts</td></tr>" << std::endl;
    	        
    	int totaldispos = 0;
    	
        for (int i = 0; i < 13; i++) {               
        	//if (i == 1) {
        		//do nothing
		//} else {
			query = "SELECT COUNT(*) FROM " + queue + " WHERE disposition = '" + itos(i) + "' ";
			if (mysql_query(mysql, query.c_str()) != 0) {
				//std::cout << "<pre>Error selecting filter!</pre>" << std::endl;
			} else {
				result = mysql_use_result(mysql);
				row = mysql_fetch_row(result);
				totaldispos = totaldispos + stoi(row[0]);
				std::cout << "<tr><td>" + itos(i) + "</td><td><font size=-1 color=\"blue\" face=\"helvetica,arial,sans\">[" + row[0] + "]</font></td></tr>";
				mysql_free_result(result);
			}
		//}
	}

        std::cout << "<tr><td>Total</td><td>" + itos(totaldispos) + "</td></tr>" << std::endl;
        
        std::cout << "</table>" << std::endl;
        
        std::cout << "<font size=-1 face=\"helvetica,arial,sans\">";
        std::cout << "<br>(these are dispositions 0-12 from the database table)";
        std::cout << "</font>";
        
    std::cout << "<br>";
  }

void doShowAllDBCounts(Post TheFields) {
  
    	std::string queue 		= TheFields.GetField("queue");
    	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
    
    	MYSQL *mysql = NULL;
    	MYSQL_RES *result;
    	MYSQL_ROW row;

    	std::string query;
    	mysql = mysql_init(NULL);
    	if (mysql == NULL) {
    		std::cerr << "MySql init failed!" << std::endl;
	}
	if (!mysql_real_connect(mysql, getMySqlHost().c_str(), \
    				getMySqlUser().c_str(), \
    				getMySqlPass().c_str(), \
    				getDbName().c_str(), \
    				3306, NULL, 0)) {
    				std::cerr << "MySql connection failed!" << std::endl;
    	}

    	printHead("All Campaign Counts");
    	std::cout << "<br><br>";
  
    	std::cout << "<div align=left><table cols=14>" << std::endl;
	std::cout << "<tr><td>Campaign/Disposition</td>";
	for (int d = -7; d < 13; d++) {
                //std::cout << "<td width=45>" << itos(d) << "</td>";
		std::cout << "<td width=45>" << dispo2short(d) << "</td>";
	}
	std::cout << "</tr>" << std::endl;
		        
        QueueList TheQueues;
  	TheQueues.ParseQueues();
        
        std::string tempCampaign;
        for (int q = 0; q < TheQueues.size(); q++) {
                tempCampaign = TheQueues.at(q).GetName();
                if (tempCampaign.find("CLOSER",0) != std::string::npos || tempCampaign.find("closer",0) != std::string::npos || tempCampaign.find("DNC",0) != std::string::npos || tempCampaign.find("dnc",0) != std::string::npos) {
    	        //if (tempCampaign == "CLOSER") {
    	        	//do nothing
		} else {
			std::cout << "<tr>";
                        std::cout << "<td>" << tempCampaign << "</td>";
    			int totaldispos = 0;
        		for (int i = -7; i < 13; i++) {               
     
				query = "SELECT COUNT(*) FROM " + tempCampaign + " WHERE disposition = '" + itos(i) + "' ";
				if (mysql_query(mysql, query.c_str()) != 0) {
					//std::cout << "<pre>Error selecting filter!</pre>" << std::endl;
				} else {
					result = mysql_use_result(mysql);
					row = mysql_fetch_row(result);
					totaldispos = totaldispos + stoi(row[0]);
					std::cout << "<td>" << row[0] << "</td>";
					mysql_free_result(result);
				}
			}
			std::cout << "</tr>";         
		}
	}
        
        std::cout << "</table>" << std::endl;
        
        std::cout << "<font size=-1 face=\"helvetica,arial,sans\">";
        std::cout << "<br>(these are dispositions -7 through 12 from the database table)";
        std::cout << "</font>";
        
        std::cout << "<br>";
}

void doShowAllDBCountsForToday(Post TheFields) {
  
    	std::string queue 		= TheFields.GetField("queue");
    	std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
    
    	MYSQL *mysql = NULL;
    	MYSQL_RES *result;
    	MYSQL_ROW row;

    	std::string query;
    	mysql = mysql_init(NULL);
    	if (mysql == NULL) {
    		std::cerr << "MySql init failed!" << std::endl;
	}
	if (!mysql_real_connect(mysql, getMySqlHost().c_str(), \
    				getMySqlUser().c_str(), \
    				getMySqlPass().c_str(), \
    				getDbName().c_str(), \
    				3306, NULL, 0)) {
    				std::cerr << "MySql connection failed!" << std::endl;
    	}

    	printHead("All Campaign Counts");
    	std::cout << "<br><br>";
  
    	std::cout << "<div align=left><table cols=14>" << std::endl;
	std::cout << "<tr><td>Campaign/Disposition</td>";
	for (int d = -7; d < 13; d++) {
                //std::cout << "<td width=45>" << itos(d) << "</td>";
		std::cout << "<td width=45>" << dispo2short(d) << "</td>";
	}
	std::cout << "</tr>" << std::endl;
		        
        QueueList TheQueues;
  	TheQueues.ParseQueues();
        
        std::string tempCampaign;
        for (int q = 0; q < TheQueues.size(); q++) {
                tempCampaign = TheQueues.at(q).GetName();
		if (tempCampaign.find("CLOSER",0) != std::string::npos || tempCampaign.find("closer",0) != std::string::npos || tempCampaign.find("DNC",0) != std::string::npos || tempCampaign.find("dnc",0) != std::string::npos) {
		//if (tempCampaign == "CLOSER" || tempCampaign == "closer" || tempCampaign == "DC_CLOSER") {
    	        	//do nothing
		} else {
			std::cout << "<tr>";
                        std::cout << "<td>" << tempCampaign << "</td>";
    			int totaldispos = 0;
        		for (int i = -7; i < 13; i++) {
				query = "SELECT COUNT(*) FROM " + tempCampaign + " WHERE disposition = '" + itos(i) + "' AND LEFT(lastupdated,10) = LEFT(NOW(),10) ";
				if (mysql_query(mysql, query.c_str()) != 0) {
					//std::cout << "<pre>Error selecting filter!</pre>" << std::endl;
				} else {
					result = mysql_use_result(mysql);
					row = mysql_fetch_row(result);
					totaldispos = totaldispos + stoi(row[0]);
					std::cout << "<td>" << row[0] << "</td>";
					mysql_free_result(result);
				}
			}
			std::cout << "</tr>";         
		}
	}
        
        std::cout << "</table>" << std::endl;
        
        std::cout << "<font size=-1 face=\"helvetica,arial,sans\">";
        std::cout << "<br>(these are dispositions -7 through 12 from the database table)";
        std::cout << "</font>";
        
        std::cout << "<br>";
}

void doEditQueue(Post TheFields) {
  
    std::string queue 		= TheFields.GetField("queue");
    std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";

    Queue TheQueue;
	u_long serverId = 1;
    TheQueue.ParseQueue(queue,serverId);

    std::string function          = TheQueue.GetSetting("function").Get();
    std::string calltoday         = TheQueue.GetSetting("calltoday").Get();
    std::string usednc	 	  = TheQueue.GetSetting("usednc").Get();
    std::string maxratio          = TheQueue.GetSetting("maxratio").Get();
    std::string maxabandons       = TheQueue.GetSetting("maxabandons").Get();
    std::string maxlines          = TheQueue.GetSetting("maxlines").Get();
    std::string sleeptime         = TheQueue.GetSetting("sleeptime").Get();
    std::string tzearliest        = TheQueue.GetSetting("tzearliest").Get();
    std::string tzlatest          = TheQueue.GetSetting("tzlatest").Get();
    std::string timeout           = TheQueue.GetSetting("timeout").Get();
    std::string dspmode           = TheQueue.GetSetting("dspmode").Get();
    std::string trunk	          = TheQueue.GetSetting("trunk").Get();
    std::string dialprefix        = TheQueue.GetSetting("dialprefix").Get();
    std::string usecloser         = TheQueue.GetSetting("usecloser").Get();
    std::string callerid          = TheQueue.GetSetting("callerid").Get();
    std::string closercam         = TheQueue.GetSetting("closercam").Get();
    std::string usecallback       = TheQueue.GetSetting("usecallback").Get();
    std::string debug		  = TheQueue.GetSetting("debug").Get();
    std::string printagentsales   = TheQueue.GetSetting("prn_agent_sales").Get();
    std::string printclosersales  = TheQueue.GetSetting("prn_closer_sales").Get();
    std::string printclosernosales = TheQueue.GetSetting("prn_closer_nosales").Get();

    if (trunk.find("%3A",0) != std::string::npos) {
    	int trunkPos = trunk.find("%3A",0);
    	trunk.erase(trunkPos,3);
    	trunk.insert(trunkPos,":");
    }
                                
    printHead("Edit Campaign");
    std::cout << "<br><br>";
    printSubHead("Enter campaign options for ");
    printSubHead(queue);
    printSubHead(".");
    std::cout << "<br><br>";
  
    std::cout << "<div align=left><table cols=2>" << std::endl;
    std::cout << "<tr><td>";
    std::cout << " Dialing Mode: ";
    std::cout << printAlert("[?]","This is where you define the method by which the dialer throttles the outgoing call volume.\\n\\nEach mode follows a different set of rules.\\nPredictive:\\nThe dialer will first check if it has reached maxlines. Then it will check if it has reached maxabandons. Finally it will check if it is over maxratio.  If none of those checks returned false, the dialer will assume that it is okay to place a call.\\nPower:\\nThe dialer will check if it is over maxlines. Then it will check if it is over maxratio.\\nPreview:\\nIdentical to power, except that the maxratio is always 1.");
    std::cout << "</td><td>";
    std::cout << "<select name=\"function\" size=1 value=\"" + function + "\">\n";        
    if (function == "predictive") std::cout << "<option SELECTED>predictive<br>\n";
      else { std::cout << "<option>predictive<br>\n"; }
    if (function == "power") std::cout << "<option SELECTED>power<br>\n";
      else { std::cout << "<option>power<br>\n"; }
    if (function == "auto") std::cout << "<option SELECTED>auto<br>\n";
      else { std::cout << "<option>auto<br>\n"; }
    if (function == "preview") std::cout << "<option SELECTED>preview<br>\n";
      else { std::cout << "<option>preview<br>\n"; }
    if (function == "inbound") std::cout << "<option SELECTED>inbound<br>\n";
      else { std::cout << "<option>inbound<br>\n"; }
    if (function == "closer") std::cout << "<option SELECTED>closer<br>\n";
      else { std::cout << "<option>closer<br>\n"; }
    std::cout << "</select>";
    std::cout << "</td></tr>";

    std::cout << "<tr><td>";
    std::cout << "Callback Today: ";
    std::cout << printAlert("[?]","If you want to call leads multiple times a day, set this to yes.\\n\\nOtherwise, Gnudialer will ONLY make one attempt a day per record.");
    std::cout << "</td><td>";
    std::cout << "<select name=\"calltoday\" size=1>";
    std::cout << "<option value=true " << selectedIf("true",calltoday) << ">Yes\n";
    std::cout << "<option value=false " << selectedIf("false",calltoday) << ">No\n";
    std::cout << "</select>";
    std::cout << "</td></tr>";


    std::cout << "<tr><td>";
    std::cout << "DNC Scrub: ";
    std::cout << printAlert("[?]","Realtime check against your DNC table.\\n\\nOnly use this feature if you actually filled the DNC table!!!");
    std::cout << "</td><td>";
    std::cout << "<select name=\"usednc\" size=1>";
    std::cout << "<option value=true " << selectedIf("true",usednc) << ">Yes\n";
    std::cout << "<option value=false " << selectedIf("false",usednc) << ">No\n";
    std::cout << "</select>";
    std::cout << "</td></tr>";
	 
    std::cout << "<tr><td>";
    std::cout << "Max Ratio: ";
    std::cout << printAlert("[?]","This is the maximum line to available agent ratio that the dialer will allow for your campaign.\\nFor example, if you set this to 2, and there are 15 agents ready to take a call,\\nthe dialer will place no more than 30 calls during that time.  \\nKeep in mind that this number may also be limited by your maxabandon percentage.");
    std::cout << "</td><td>";
    std::cout << "<input type=text name=\"maxratio\" size=7 value=\"" + maxratio + "\">";
    std::cout << "</td></tr>";

    std::cout << "<tr><td>";
    std::cout << "Max Abandon \%: ";
    std::cout << printAlert("[?]","This is the maximum number of abandons divided by calls to allow \\nat any given point in time.  \\n(Campaigns covered under the TSR should be set at 3.0 percent or lower)");
    std::cout << "</td><td>";
    std::cout << "<input type=text name=\"maxabandons\" size=7  value=\"" + maxabandons + "\">";
    std::cout << "</td></tr>";
    
    std::cout << "<tr><td>";
    std::cout << "Max Outgoing Lines: ";
    std::cout << printAlert("[?]","This is the maximum number of outgoing lines to use for your campaign.\\nThe dialer will not call any leads when this threshhold has been reached.");
    std::cout << "</td><td>";
    std::cout << "<input type=text name=\"maxlines\" size=7  value=\"" + maxlines + "\">";
    std::cout << "</td></tr>";

    std::cout << "<tr><td>";    
    std::cout << "Query Sleep Time: ";
    std::cout << printAlert("[?]","This is how often (in milliseconds) that the dialer should check to see if \\nit is appropriate to make another call.  The lower the number, \\nthe higher the strain on your processor.");
    std::cout << "</td><td>";
    std::cout << "<input type=text name=\"sleeptime\" size=7  value=\"" + sleeptime + "\">";
    std::cout << "</td></tr>";
    std::cout << "<tr><td>";
    
	std::cout << "Call between: ";
	std::cout << printAlert("[?]","This is the time range to call the leads.\\nThe dialer will not call a lead earlier than the first time,\\nwill not call later than the last time.");
	std::cout << "</td><td>";
        std::cout << "<select name=\"tzearliest\" size=1 value=\"" + tzearliest + "\">\n";
        if (tzearliest == "0") std::cout << "<option value=0 SELECTED>12 AM<br>\n";
                  else { std::cout << "<option value=0>12 AM<br>\n"; }
        if (tzearliest == "1") std::cout << "<option value=1 SELECTED>1 AM<br>\n";
                  else { std::cout << "<option value=1>1 AM<br>\n"; }
        if (tzearliest == "2") std::cout << "<option value=2 SELECTED>2 AM<br>\n";
                  else { std::cout << "<option value=2>2 AM<br>\n"; }
        if (tzearliest == "3") std::cout << "<option value=3 SELECTED>3 AM<br>\n";
                  else { std::cout << "<option value=3>3 AM<br>\n"; }
        if (tzearliest == "4") std::cout << "<option value=4 SELECTED>4 AM<br>\n";
                  else { std::cout << "<option value=4>4 AM<br>\n"; }
        if (tzearliest == "5") std::cout << "<option value=5 SELECTED>5 AM<br>\n";
                  else { std::cout << "<option value=5>5 AM<br>\n"; }
        if (tzearliest == "6") std::cout << "<option value=6 SELECTED>6 AM<br>\n";
		  else { std::cout << "<option value=6>6 AM<br>\n"; }
        if (tzearliest == "7") std::cout << "<option value=7 SELECTED>7 AM<br>\n";
                  else { std::cout << "<option value=7>7 AM<br>\n"; }
        if (tzearliest == "8") std::cout << "<option value=8 SELECTED>8 AM<br>\n";
                  else { std::cout << "<option value=8>8 AM<br>\n"; }
        if (tzearliest == "9") std::cout << "<option value=9 SELECTED>9 AM<br>\n";
                  else { std::cout << "<option value=9>9 AM<br>\n"; }
        if (tzearliest == "10") std::cout << "<option value=10 SELECTED>10 AM<br>\n";
                  else { std::cout << "<option value=10>10 AM<br>\n"; }
        if (tzearliest == "11") std::cout << "<option value=11 SELECTED>11 AM<br>\n";
                  else { std::cout << "<option value=11>11 AM<br>\n"; }
        if (tzearliest == "12") std::cout << "<option value=12 SELECTED>12 PM<br>\n";
                  else { std::cout << "<option value=12>12 PM<br>\n"; }
        if (tzearliest == "13") std::cout << "<option value=13 SELECTED>1 PM<br>\n";
		  else { std::cout << "<option value=13>1 PM<br>\n"; }
        if (tzearliest == "14") std::cout << "<option value=14 SELECTED>2 PM<br>\n";
                  else { std::cout << "<option value=14>2 PM<br>\n"; }
        if (tzearliest == "15") std::cout << "<option value=15 SELECTED>3 PM<br>\n";
                  else { std::cout << "<option value=15>3 PM<br>\n"; }
        if (tzearliest == "16") std::cout << "<option value=16 SELECTED>4 PM<br>\n";
                  else { std::cout << "<option value=16>4 PM<br>\n"; }
        if (tzearliest == "17") std::cout << "<option value=17 SELECTED>5 PM<br>\n";
                  else { std::cout << "<option value=17>5 PM<br>\n"; }
        if (tzearliest == "18") std::cout << "<option value=18 SELECTED>6 PM<br>\n";
                  else { std::cout << "<option value=18>6 PM<br>\n"; }
        if (tzearliest == "19") std::cout << "<option value=19 SELECTED>7 PM<br>\n";
                  else { std::cout << "<option value=19>7 PM<br>\n"; }                 
        if (tzearliest == "20") std::cout << "<option value=20 SELECTED>8 PM<br>\n";
                  else { std::cout << "<option value=20>8 PM<br>\n"; }
        if (tzearliest == "21") std::cout << "<option value=21 SELECTED>9 PM<br>\n";
		  else { std::cout << "<option value=21>9 PM<br>\n"; }
        if (tzearliest == "22") std::cout << "<option value=22 SELECTED>10 PM<br>\n";
                  else { std::cout << "<option value=22>10 PM<br>\n"; }
        if (tzearliest == "23") std::cout << "<option value=23 SELECTED>11 PM<br>\n";
                  else { std::cout << "<option value=23>11 PM<br>\n"; }
        if (tzearliest == "24") std::cout << "<option value=24 SELECTED>12 PM<br>\n";
                  else { std::cout << "<option value=24>12 PM<br>\n"; }
	std::cout << "</select> - \n";
	std::cout << "<select name=\"tzlatest\" size=1 value=\"" + tzlatest + "\">\n";
        if (tzlatest == "0") std::cout << "<option value=0 SELECTED>12 AM<br>\n";
                  else { std::cout << "<option value=0>12 AM<br>\n"; }
        if (tzlatest == "1") std::cout << "<option value=1 SELECTED>1 AM<br>\n";
                  else { std::cout << "<option value=1>1 AM<br>\n"; }
        if (tzlatest == "2") std::cout << "<option value=2 SELECTED>2 AM<br>\n";
                  else { std::cout << "<option value=2>2 AM<br>\n"; }
        if (tzlatest == "3") std::cout << "<option value=3 SELECTED>3 AM<br>\n";
                  else { std::cout << "<option value=3>3 AM<br>\n"; }
        if (tzlatest == "4") std::cout << "<option value=4 SELECTED>4 AM<br>\n";
                  else { std::cout << "<option value=4>4 AM<br>\n"; }
        if (tzlatest == "5") std::cout << "<option value=5 SELECTED>5 AM<br>\n";
                  else { std::cout << "<option value=5>5 AM<br>\n"; }
        if (tzlatest == "6") std::cout << "<option value=6 SELECTED>6 AM<br>\n";
                  else { std::cout << "<option value=6>6 AM<br>\n"; }
        if (tzlatest == "7") std::cout << "<option value=7 SELECTED>7 AM<br>\n";
                  else { std::cout << "<option value=7>7 AM<br>\n"; }
        if (tzlatest == "8") std::cout << "<option value=8 SELECTED>8 AM<br>\n";
                  else { std::cout << "<option value=8>8 AM<br>\n"; }
        if (tzlatest == "9") std::cout << "<option value=9 SELECTED>9 AM<br>\n";
                  else { std::cout << "<option value=9>9 AM<br>\n"; }
        if (tzlatest == "10") std::cout << "<option value=10 SELECTED>10 AM<br>\n";
                  else { std::cout << "<option value=10>10 AM<br>\n"; }
        if (tzlatest == "11") std::cout << "<option value=11 SELECTED>11 AM<br>\n";
                  else { std::cout << "<option value=11>11 AM<br>\n"; }
        if (tzlatest == "12") std::cout << "<option value=12 SELECTED>12 PM<br>\n";
                  else { std::cout << "<option value=12>12 PM<br>\n"; }
        if (tzlatest == "13") std::cout << "<option value=13 SELECTED>1 PM<br>\n";
                  else { std::cout << "<option value=13>1 PM<br>\n"; }
        if (tzlatest == "14") std::cout << "<option value=14 SELECTED>2 PM<br>\n";
                  else { std::cout << "<option value=14>2 PM<br>\n"; }
        if (tzlatest == "15") std::cout << "<option value=15 SELECTED>3 PM<br>\n";
                  else { std::cout << "<option value=15>3 PM<br>\n"; }
        if (tzlatest == "16") std::cout << "<option value=16 SELECTED>4 PM<br>\n";
                  else { std::cout << "<option value=16>4 PM<br>\n"; }
        if (tzlatest == "17") std::cout << "<option value=17 SELECTED>5 PM<br>\n";
                  else { std::cout << "<option value=17>5 PM<br>\n"; }
        if (tzlatest == "18") std::cout << "<option value=18 SELECTED>6 PM<br>\n";
                  else { std::cout << "<option value=18>6 PM<br>\n"; }
        if (tzlatest == "19") std::cout << "<option value=19 SELECTED>7 PM<br>\n";
                  else { std::cout << "<option value=19>7 PM<br>\n"; }
        if (tzlatest == "20") std::cout << "<option value=20 SELECTED>8 PM<br>\n";
                  else { std::cout << "<option value=20>8 PM<br>\n"; }
        if (tzlatest == "21") std::cout << "<option value=21 SELECTED>9 PM<br>\n";
                  else { std::cout << "<option value=21>9 PM<br>\n"; }
        if (tzlatest == "22") std::cout << "<option value=22 SELECTED>10 PM<br>\n";
                  else { std::cout << "<option value=22>10 PM<br>\n"; }
        if (tzlatest == "23") std::cout << "<option value=23 SELECTED>11 PM<br>\n";
                  else { std::cout << "<option value=23>11 PM<br>\n"; }
        if (tzlatest == "24") std::cout << "<option value=24 SELECTED>12 PM<br>\n";
                  else { std::cout << "<option value=24>12 PM<br>\n"; }
	std::cout << "</select>";
	std::cout << "</td></tr>";
	
	std::cout << "<tr><td>";
	std::cout << "Timeout: ";
	std::cout << printAlert("[?]","This is the number of rings (approximately), the outgoing call will wait for an answer.\\n(each ring is set at 5000 ms, so for every ring add 5000, so 4 Rings is 20000 ms)");
        std::cout << "</td><td>";
 
//	std::cout << "<input type=text name=\"timeout\" size=7  value=\"" + timeout + "\">";
//        std::cout << "</td></tr>";
        
        std::cout << "<select name=\"timeout\" size=1 value=\"" + timeout + "\">\n";
        if (timeout == "5000") std::cout << "<option value=5000 SELECTED>1 Ring<br>\n";
                  else { std::cout << "<option value=5000>1 Ring<br>\n"; }
        if (timeout == "10000") std::cout << "<option value=10000 SELECTED>2 Rings<br>\n";
                  else { std::cout << "<option value=10000>2 Rings<br>\n"; }
        if (timeout == "15000") std::cout << "<option value=15000 SELECTED>3 Rings<br>\n";
                  else { std::cout << "<option value=15000>3 Rings<br>\n"; }
        if (timeout == "20000") std::cout << "<option value=20000 SELECTED>4 Rings<br>\n";
                  else { std::cout << "<option value=20000>4 Rings<br>\n"; }
        if (timeout == "25000") std::cout << "<option value=25000 SELECTED>5 Rings<br>\n";
                  else { std::cout << "<option value=25000>5 Rings<br>\n"; }
        if (timeout == "30000") std::cout << "<option value=30000 SELECTED>6 Rings<br>\n";
                  else { std::cout << "<option value=30000>6 Rings<br>\n"; }
        if (timeout == "35000") std::cout << "<option value=35000 SELECTED>7 Rings<br>\n";
                  else { std::cout << "<option value=35000>7 Rings<br>\n"; }
        if (timeout == "40000") std::cout << "<option value=40000 SELECTED>8 Rings<br>\n";
                  else { std::cout << "<option value=40000>8 Rings<br>\n"; }
        std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
	std::cout << "CallerID: ";
	std::cout << printAlert("[?]","CallerID NUMBER (digits only) to use for dialing out.\\n0123456789 will send the callerid as the number you are calling.\\n0123459999 will send the areacode and prefix plus 9999 as the number you are calling.");
        std::cout << "</td><td>";
	std::cout << "<input type=text name=\"callerid\" size=10 value=\"" + callerid + "\">";
        std::cout << "</td></tr>";
        
        std::cout << "<tr><td>";
	std::cout << "DSP Mode: ";
	std::cout << printAlert("[?]","This is the method by which GnuDialer does voicemail/fax detection.\\nThere are 3 main methods\\n\\nWaitForSilence\\n  WaitForSilence(x[|y]) Wait for Silence: Waits for up to \\'x\\' \\nmilliseconds of silence, \\'y\\' times or 1 if omitted\\n\\nBG Detect\\n  BackgroundDetect(filename[|sil[|min|[max]]]):  Plays back a given filename\\nchecking min/max silence times\\n\\nNV Fax (and VM) Detect\\n  NVFaxDetect([waitdur[|options[|sildur[|mindur[|maxdur]]]]]): This application \\nlistens for fax tones (on IAX and SIP channels too) for waitdur seconds of time.\\n(this one requires contacting Newman Telecom at info@newmantelecom.com\\nto get the applications to add to asterisk)");
        std::cout << "</td><td>";
	std::cout << "<select name=\"dspmode\" size=1>";
	std::cout << "<option value=none " << selectedIf("none",dspmode) << ">None\n";
	std::cout << "<option value=waitforsilence " << selectedIf("waitforsilence",dspmode) << ">Wait For Silence\n";
	std::cout << "<option value=backgrounddetect " << selectedIf("backgrounddetect",dspmode) << ">BG Detect\n";
	std::cout << "<option value=nvfaxdetect " << selectedIf("nvfaxdetect",dspmode) << ">NV Fax (and VM) Detect\n";
        std::cout << "<option value=custom " << selectedIf("custom",dspmode) << ">Custom\n";
	std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Trunk: ";
        std::cout << printAlert("[?]","This is the outgoing trunk to use for this campaigns dialing.\\n\\nExamples: \\nZap/g1\\nIAX2/username:password@provider\\n");
        std::cout << "</td><td>";
        std::cout << "<input type=text name=\"trunk\" size=30 value=\"" + trunk + "\">";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Dial Prefix: ";
        std::cout << printAlert("[?]","This is an optional dialing prefix that is dialed before\\nthe phone number from the database.\\n\\nExample: 9 OR 011 ");
        std::cout << "</td><td>";
        std::cout << "<input type=text name=\"dialprefix\" size=20 value=\"" + dialprefix + "\">";
        std::cout << "</td></tr>";


        std::cout << "<tr><td>";
        std::cout << "Use Callback: ";
        std::cout << printAlert("[?]","Allow the ability to set a callback.");
        std::cout << "</td><td>";
        std::cout << "<select name=\"usecallback\" size=1>";
        std::cout << "<option value=true " << selectedIf("true",usecallback) << ">Yes\n";
        std::cout << "<option value=false " << selectedIf("false",usecallback) << ">No\n";
        std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Allow Closer: ";
        std::cout << printAlert("[?]","Allow agent to transfer to a closer.");
        std::cout << "</td><td>";
        std::cout << "<select name=\"usecloser\" size=1>";
        std::cout << "<option value=true " << selectedIf("true",usecloser) << ">Yes\n";
        std::cout << "<option value=false " << selectedIf("false",usecloser) << ">No\n";
        std::cout << "</select>";
        std::cout << "</td></tr>";

	QueueList TheQueues;
	TheQueues.ParseQueues();
        
        std::cout << "<tr><td>";
        std::cout << "Closer Campaign: ";
        std::cout << printAlert("[?]","This is the campaign to send transfers to.");
        std::cout << "</td><td>";
	std::cout << "<select name=\"closercam\" size=1>";
	std::cout << "<option " << selectedIf("none",closercam) << ">none\n";
	for (int i = 0; i < TheQueues.size(); i++) {
		std::cout << "<option " << selectedIf(TheQueues.at(i).GetName(),closercam) << ">" << TheQueues.at(i).GetName() << "\n";
		}
	std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Debug: ";
        std::cout << printAlert("[?]","Send Campaign debug output to the console.");
        std::cout << "</td><td>";
        std::cout << "<select name=\"debug\" size=1>";
        std::cout << "<option value=true " << selectedIf("true",debug) << ">Yes\n";
        std::cout << "<option value=false " << selectedIf("false",debug) << ">No\n";
        std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Print Agent Sales: ";
        std::cout << printAlert("[?]","(if cups/lpr configured) Print the record an Agent dispositions a 12 (sale).");
        std::cout << "</td><td>";
        std::cout << "<select name=\"printagentsales\" size=1>";
        std::cout << "<option value=true " << selectedIf("true",printagentsales) << ">Yes\n";
        std::cout << "<option value=false " << selectedIf("false",printagentsales) << ">No\n";
        std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Print Closer Sales: ";
        std::cout << printAlert("[?]","(if cups/lpr configured) Print the record a Closer dispositions a 12 (sale).");
        std::cout << "</td><td>";
        std::cout << "<select name=\"printclosersales\" size=1>";
        std::cout << "<option value=true " << selectedIf("true",printclosersales) << ">Yes\n";
        std::cout << "<option value=false " << selectedIf("false",printclosersales) << ">No\n";
        std::cout << "</select>";
        std::cout << "</td></tr>";

        std::cout << "<tr><td>";
        std::cout << "Print Closer NoSales: ";
        std::cout << printAlert("[?]","(if cups/lpr configured) Print the record a Closer dispositions an 11 (nosale).");
        std::cout << "</td><td>";
        std::cout << "<select name=\"printclosernosales\" size=1>";
        std::cout << "<option value=true " << selectedIf("true",printclosernosales) << ">Yes\n";
        std::cout << "<option value=false " << selectedIf("false",printclosernosales) << ">No\n";
        std::cout << "</select>";
        std::cout << "</td></tr>";


        std::cout << "</table></div>";
        std::cout << "<br>";
            
	std::cout << printSubmit("Save Campaign Settings");
 
        std::cout << "<font size=-1 face=\"helvetica,arial,sans\">";
        std::cout << "<br>(changes take effect the next Update cycle (5-20 secs))";
        std::cout << "</font>";
        
    std::cout << "<br>";
  }
  
void doFinishEditingCampaignSettings(Post TheFields) {

  std::string queue             = TheFields.GetField("queue");
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
  std::string function          = TheFields.GetField("function");
  std::string calltoday	  	= TheFields.GetField("calltoday");
  std::string usednc  	        = TheFields.GetField("usednc");
  std::string maxratio          = TheFields.GetField("maxratio");
  std::string maxabandons       = TheFields.GetField("maxabandons");
  std::string maxlines          = TheFields.GetField("maxlines");
  std::string sleeptime         = TheFields.GetField("sleeptime");
  std::string tzearliest        = TheFields.GetField("tzearliest");
  std::string tzlatest          = TheFields.GetField("tzlatest");
  std::string timeout           = TheFields.GetField("timeout");
  std::string dspmode           = TheFields.GetField("dspmode");
  std::string trunk             = TheFields.GetField("trunk");
  std::string dialprefix        = TheFields.GetField("dialprefix");
  std::string usecloser         = TheFields.GetField("usecloser");
  std::string callerid          = TheFields.GetField("callerid");
  std::string closercam         = TheFields.GetField("closercam");
  std::string usecallback       = TheFields.GetField("usecallback");
  std::string debug	        = TheFields.GetField("debug");
  std::string printagentsales  = TheFields.GetField("printagentsales");
  std::string printclosersales  = TheFields.GetField("printclosersales");
  std::string printclosernosales = TheFields.GetField("printclosernosales");
  
  if (maxratio.empty()) {
        maxratio = "2.0";
  }
  if (maxabandons.empty()) {
        maxabandons = "3.0";
  }
  if (maxlines.empty()) {
        maxlines = "24";
  }
  if (sleeptime.empty()) {
        sleeptime = "1000";
  }
  if (callerid.empty()) {
        callerid = "9999999999";
  }
  if (trunk.find(":",0) != std::string::npos) {
  	int trunkPos = trunk.find(":",0);
  	trunk.erase(trunkPos,1);
  	trunk.insert(trunkPos,"%3A");
  }
  if (trunk.empty()) {
        trunk = "none";
  }
  if (dialprefix.empty()) {
        dialprefix = "none";
  }
  Queue TheQueue;
  u_long serverId = 1;
  TheQueue.ParseQueue(queue,serverId);

  TheQueue.SetSetting("function",		function);
  TheQueue.SetSetting("calltoday",              calltoday);
  TheQueue.SetSetting("usednc",	  	        usednc);
  TheQueue.SetSetting("maxratio",		maxratio);
  TheQueue.SetSetting("maxabandons",		maxabandons);
  TheQueue.SetSetting("maxlines",		maxlines);
  TheQueue.SetSetting("sleeptime",		sleeptime);
  TheQueue.SetSetting("tzearliest",		tzearliest);
  TheQueue.SetSetting("tzlatest",		tzlatest);
  TheQueue.SetSetting("timeout",        	timeout);
  TheQueue.SetSetting("dspmode",        	dspmode);
  TheQueue.SetSetting("trunk",                  trunk);
  TheQueue.SetSetting("dialprefix",             dialprefix);
  TheQueue.SetSetting("usecloser",      	usecloser);
  TheQueue.SetSetting("closercam",      	closercam);
  TheQueue.SetSetting("callerid",       	callerid);
  TheQueue.SetSetting("usecallback",    	usecallback);
  TheQueue.SetSetting("debug",    		debug);
  TheQueue.SetSetting("prn_agent_sales",        printagentsales);
  TheQueue.SetSetting("prn_closer_sales",       printclosersales);
  TheQueue.SetSetting("prn_closer_nosales",     printclosernosales);

  TheQueue.Write();

  printHead("Save Campaign Settings");
  std::cout << "<br><br>\n";
  printSubHead("Settings updated for " + queue);

  std::cout << "<br><br>";
  }                                  

void doStartQueue(Post TheFields) {

  std::string queue = TheFields.GetField("queue");
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
  
//  Queue TheQueue;
// u_long serverId = 1;
//  TheQueue.ParseQueue(queue,serverId);
 
//  std::string function		= TheQueue.GetSetting("function").Get();
//  std::string maxratio 		= TheQueue.GetSetting("maxratio").Get();
//  std::string maxabandons 	= TheQueue.GetSetting("maxabandons").Get();
//  std::string maxlines 		= TheQueue.GetSetting("maxlines").Get();
//  std::string sleeptime 	= TheQueue.GetSetting("sleeptime").Get();
//  std::string tzearliest 	= TheQueue.GetSetting("tzearliest").Get();
//  std::string tzlatest 		= TheQueue.GetSetting("tzlatest").Get();
//  std::string timeout           = TheQueue.GetSetting("timeout").Get();

//  bool success = startQueue(queue);
  startQueue(queue);
  
  //sendUserEvent("StartCampaign","Campaign: " + queue);

  printHead("Campaign Started");
  std::cout << "<br><br>\n";
  printSubHead("You have started campaign ");
  printSubHead(queue);
  printSubHead(".");
  std::cout << "<br><br>\n";

//  std::cout << "Details:\n\n";
 
//  std::cout << "<div align=left><table cols=2>" << std::endl;
//  std::cout << "<tr><td>Mode:</td><td>"	<< function << "</td></tr>" << std::endl;
//  std::cout << "<tr><td>Line/Station Ratio:</td><td>" << maxratio << "</td></tr>" << std::endl;
//  std::cout << "<tr><td>Max Abandon \%:</td><td>" << maxabandons << "</td></tr>" << std::endl;
//  std::cout << "<tr><td>Max Lines to Use:</td><td>" << maxlines << "</td></tr>" << std::endl;
//  std::cout << "<tr><td>Query Sleep Time:</td><td>" << sleeptime << "</td></tr>" << std::endl;
//  std::cout << "<tr><td>Earliest Time:</td><td>" << tzearliest << "</td></tr>" << std::endl;
//  std::cout << "<tr><td>Latest Time:</td><td>" << tzlatest << "</td></tr>" << std::endl;
//  std::cout << "</table></div>" << std::endl;
//    std::cout << "<br><br>";

  }

void doStopQueue(Post TheFields) {

  std::string queue = TheFields.GetField("queue");
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
  Queue TheQueue;
  u_long serverId = 1;
  TheQueue.ParseQueue(queue,serverId);
 
//  bool success = stopQueue(queue);
  stopQueue(queue);	
  //sendUserEvent("StopCampaign","Campaign: " + queue);
  

  printHead("Campaign Started");
  std::cout << "<br><br>\n";
  printSubHead("You have stopped campaign ");
  printSubHead(queue);
  printSubHead(".");
  std::cout << "<br><br>\n";

  }


void doRemoveQueue(Post TheFields) {

  std::string queue = TheFields.GetField("queue");
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
  removeQueue(queue);

  //sendUserEvent("DeletedCampaign","Campaign: " + queue);
  
  printHead("Campaign Queue");
  std::cout << "<br><br>\n";
  printSubHead("You have removed campaign<br><br>");
  printSubHead(queue);
  printSubHead(".<br><br>");

  }

void doFinishCreatingQueue(Post TheFields) {

  std::string queue = TheFields.GetField("newqueuename");
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
  if (!createQueue(queue)) std::cout << "Error creating campaign!";
  else {

    addBasicSettings(queue);
    
    std::string filename = "/var/www/cgi-bin/" + queue + ".script";
                                                                                                                    
    std::system(("cp /var/www/cgi-bin/default.script " + filename).c_str());
 
    //sendUserEvent("CreatedCampaign","Campaign: " + queue);
 
    printHead("Created Campaign");
    std::cout << "<br><br>\n";
    printSubHead("You have created campaign ");
    printSubHead(queue);
    printSubHead(".<br><br>");

    }

  }

void doCreateQueue(Post TheFields) {

  std::string queue = TheFields.GetField("queue");
  std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
  
  printHead("Create Campaign");
  std::cout << "<br><br>\n";
  printSubHead("Enter the name of the campaign to create.");
  std::cout << "<br>NO dashes, spaces, etc (except '_' an underscore)";
  std::cout << "<br>Must contain 'TAP' for tap campaigns.<br><br>";
  
  std::cout << "<input type=\"text\" name=\"newqueuename\" size=\"30\" maxsize=\"30\"><br><br>\n";
  std::cout << printSubmit("Finish Creating Campaign");
  std::cout << "<br><br>\n";

  }

void doFinishCreatingAgent(Post TheFields) {

  std::string agent = TheFields.GetField("newagentname");
  std::string pass = TheFields.GetField("newagentpass");

  if (!createAgent(agent,pass)) std::cout << "Error creating agent!";
  else {

    reloadAgents();
    printHead("Created Agent");
    std::cout << "<br><br>\n";
    printSubHead("You have created agent ");
    printSubHead(agent);
    printSubHead(".<br><br>");

    }

  }

void doDeleteAgent(Post TheFields) {

  std::string queue = TheFields.GetField("queue");
  
  printHead("Delete Agents");
  std::cout << "<br><br>";
  printSubHead("You deleted <br><br>");
        
  for (int i = 0; i < TheFields.OccurencesOf("agents"); i++) {
    std::string agent = TheFields.GetField("agents",i);
    if (deleteAgent(agent)) {
      reloadAgents();    
      printSubHead(agent);
      printSubHead("<br>");
    }
  }
                        
  std::cout << "<br><br>\n";
                                
}

void doFinishCreatingCloser(Post TheFields) {

  std::string agent = TheFields.GetField("newagentname");
  std::string pass = TheFields.GetField("newagentpass");

  if (!createCloser(agent,pass)) std::cout << "Error creating agent!";
  else {

    reloadAgents();
    printHead("Created Closer");
    std::cout << "<br><br>\n";
    printSubHead("You have created closer ");
    printSubHead(agent);
    printSubHead(".<br><br>");

    }

  }

void doCreateCloser() {

  printHead("Create Closer");
  std::cout << "<br><br>\n";
  printSubHead("Enter the name of the closer to create.");
  std::cout << "<br><br>";

  std::cout << "<input type=\"text\" name=\"newagentname\"><br><br>\n";
  printSubHead("Enter a password.");
  std::cout << "<br><br>";
  std::cout << "<input type=\"password\" name=\"newagentpass\"><br><br>\n";
  std::cout << printSubmit("Finish Creating Closer");
  std::cout << "<br><br>\n";

  }

void doCreateAgent() {

  printHead("Create Agent");
  std::cout << "<br><br>\n";
  printSubHead("Enter the name of the agent to create.");
  std::cout << "<br><br>";

  std::cout << "<input type=\"text\" name=\"newagentname\"><br><br>\n";
  printSubHead("Enter a password.");
  std::cout << "<br><br>";
  std::cout << "<input type=\"password\" name=\"newagentpass\"><br><br>\n";
  std::cout << printSubmit("Finish Creating Agent");
  std::cout << "<br><br>\n";

  }

void doEditAgentInfo(Post TheFields) {

  std::string agent = TheFields.GetField("agents");
  AgentList TheAgents;
  TheAgents.ParseAgentList();
  Agent TheAgent;
  for (int i = 0; i < TheAgents.size(); i++) {
    if (TheAgents.at(i).GetNumber() == atoi(agent.c_str())) {
      TheAgent = TheAgents.at(i);
      }
    }

  printHead("Edit Agent Info");
  std::cout << "<br><br>\n";
  printSubHead("Update the agent's information:");
  std::cout << "<br><br>";
  std::cout << "Agent Name:<br><br>\n";
  std::cout << "<input type=hidden name=\"agentnumber\" value=\"" + agent + "\">\n";
  std::cout << "<input type=text name=\"editagentname\" value=\"" + TheAgent.GetName() + "\"><br><br>\n";
  std::cout << "Agent Password:<br><br>\n";
  std::cout << "<input type=password name=\"editagentpass\" value=\"" + TheAgent.GetPass() + "\"><br><br>\n";
  std::cout << printSubmit("Finish Editing Agent");
  std::cout << "<br><br>\n";

  }

void doFinishEditingAgent(Post TheFields) {

  std::string agentnumber = TheFields.GetField("agentnumber");
  std::string agentname = TheFields.GetField("editagentname");
  std::string agentpass = TheFields.GetField("editagentpass");

  if (editAgent(agentnumber,agentname,agentpass)) {

    reloadAgents();
    printHead("Edit Agent Info");
    std::cout << "<br><br>\n";
    printSubHead("You have updated agent: " + agentnumber);

    }

  std::cout << "<br><br>";

  }

void doShowAgentStats(Post TheFields) {

	std::string agent = TheFields.GetField("agents");

	printHead("Agent: " + agent);
	std::cout << "<br><br>" << std::endl;

	printSubHead("Date Range");

	std::cout << "<br><br>" << std::endl;

	printDateFields(1);

	std::cout << "<br><br>To<br><br>\n";

	printDateFields(2);

	std::cout << "<br><br>" << std::endl;
        std::cout << "Dispos to show:<br><br>";
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=callback>      Callbacks              <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=noanswer>      No Answers             <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=voicemail>     Voicemails             <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=busy>          Busies                 <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=fastbusy>      Fast Busies            <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=fax>           Fax Machines           <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=disconnect>    Disconnects            <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=donotcall>     Do Not Calls           <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=invalid>       Invalids               <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=nosalefalse>   No Sales (No Contact)  <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=nosaletrue>    No Sales (Contact)     <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=sale>          Sales                  <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=used>          Used                   <br>" << std::endl;
        std::cout << "<br>" << std::endl;

        std::cout << "<input type=hidden name=selection value=\"" << agent << "\">\n";   
        std::cout << "<input type=hidden name=statmode value=agent>\n";

        std::cout << printSubmit("Show Stats");
        std::cout << "<br><br>";

	}

void doShowAllCampaignStats(Post TheFields) {

	std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
	printHead("Campaign: " + queue);
	std::cout << "<br><br>" << std::endl;

	printSubHead("Date Range");

	std::cout << "<br><br>" << std::endl;

	printDateFields(1);

	std::cout << "<br><br>To<br><br>\n";

	printDateFields(2);

	std::cout << "<br><br>" << std::endl;

	std::cout << "Dispos to show:<br><br>";
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=callback>      Callbacks              <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=noanswer>      No Answers             <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=voicemail>     Voicemails             <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=busy>          Busies                 <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=fastbusy>      Fast Busies            <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=fax>           Fax Machines           <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=disconnect>    Disconnects            <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=donotcall>     Do Not Calls           <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=invalid>       Invalids               <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=nosalefalse>   No Sales (No Contact)  <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=nosaletrue>    No Sales (Contact)     <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=sale>          Sales                  <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=used>          Used                   <br>" << std::endl;
	std::cout << "<br>" << std::endl;

	std::cout << "<input type=hidden name=selection value=\"" << queue << "\">\n";
	std::cout << "<input type=hidden name=statmode value=allcampaign>\n";

	std::cout << printSubmit("Show Stats");
	std::cout << "<br><br>";

	}

void doShowSystemStats(Post TheFields) {

	std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
	printHead("Campaign: " + queue);
	std::cout << "<br><br>" << std::endl;

	printSubHead("Date Range");

	std::cout << "<br><br>" << std::endl;

	printDateFields(1);

	std::cout << "<br><br>To<br><br>\n";

	printDateFields(2);

	std::cout << "<br><br>" << std::endl;

	std::cout << "<input type=hidden name=selection value=\"" << queue << "\">\n";
	std::cout << "<input type=hidden name=statmode value=system>\n";

	std::cout << printSubmit("Show System Stats");
	std::cout << "<br><br>";

	}


void doShowCampaignStats(Post TheFields) {

	std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
	printHead("Campaign: " + queue);
	std::cout << "<br><br>" << std::endl;

	printSubHead("Date Range");

	std::cout << "<br><br>" << std::endl;

	printDateFields(1);

	std::cout << "<br><br>To<br><br>\n";

	printDateFields(2);

	std::cout << "<br><br>" << std::endl;

	std::cout << "Dispos to show:<br><br>";
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=callback>      Callbacks              <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=noanswer>      No Answers             <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=voicemail>     Voicemails		    <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=busy>          Busies                 <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=fastbusy>      Fast Busies            <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=fax>           Fax Machines           <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=disconnect>    Disconnects            <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=donotcall>     Do Not Calls           <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=invalid>       Invalids               <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=nosalefalse>   No Sales (No Contact)  <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=nosaletrue>    No Sales (Contact)     <br>" << std::endl;
	std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=sale>          Sales                  <br>" << std::endl;
        std::cout << "&nbsp;&nbsp;<input type=checkbox checked name=used>          Used                   <br>" << std::endl;
	std::cout << "<br>" << std::endl;

	std::cout << "<input type=hidden name=selection value=\"" << queue << "\">\n";
	std::cout << "<input type=hidden name=statmode value=campaign>\n";

	std::cout << printSubmit("Show Stats");
	std::cout << "<br><br>";

	}

void doShowAgentTime() {

        printHead("Log Time for All Agents");
        std::cout << "<br><br>" << std::endl;

        printSubHead("Date Range");

        std::cout << "<br><br>" << std::endl;

        printDateFields(1);

        std::cout << "<br><br>To<br><br>\n";

        printDateFields(2);

        std::cout << "<br><br>" << std::endl;

        std::cout << "<input type=hidden name=selection value=\"notapplicable\">\n";
        std::cout << "<input type=hidden name=statmode value=allagents>\n";

        std::cout << printSubmit("Show Stats");
        std::cout << "<br><br>";

        }

void doShowSysStats(Post TheFields) {

	std::string thestatsurl;

	printSubHead("Generating report...");

        thestatsurl = "/cgi-bin/sysstats.cgi?statsmode=" + TheFields.GetField("statmode") + "&selection=" +  TheFields.GetField("selection");
 
	std::cout << "<br><br>\n";

	thestatsurl += "&mon1=" + TheFields.GetField("mon1");
	thestatsurl += "&day1=" + TheFields.GetField("day1");
	thestatsurl += "&year1=" + TheFields.GetField("year1");

	thestatsurl += "&mon2=" + TheFields.GetField("mon2");
	thestatsurl += "&day2=" + TheFields.GetField("day2");
	thestatsurl += "&year2=" + TheFields.GetField("year2");

        std::cout << "<meta http-equiv=\"refresh\" content=\"1;url=" << thestatsurl << "\">\n\n";

	std::cout << "<a href=\"" << thestatsurl << "\">Click here</a> if you are not redirected." << std::endl;

	std::cout << "<br><br>\n";

	}

void doShowStats(Post TheFields) {

	std::string thestatsurl;

	printSubHead("Generating report...");

        if (TheFields.GetField("statmode") == "allagents") {

                thestatsurl = "/cgi-bin/agenthours.cgi?statsmode=" + TheFields.GetField("statmode") + "&selection=notapplicable";

        } else {

                thestatsurl = "/cgi-bin/newstats.cgi?statsmode=" + TheFields.GetField("statmode") + "&selection=" +  TheFields.GetField("selection");

                }

	if (TheFields.HasField("callback"))     thestatsurl += "&callback=on";
        if (TheFields.HasField("noanswer"))     thestatsurl += "&noanswer=on";
        if (TheFields.HasField("voicemail"))    thestatsurl += "&voicemail=on";
        if (TheFields.HasField("busy"))         thestatsurl += "&busy=on";
        if (TheFields.HasField("fastbusy"))     thestatsurl += "&fastbusy=on";
        if (TheFields.HasField("fax"))          thestatsurl += "&fax=on";
        if (TheFields.HasField("disconnect"))   thestatsurl += "&disconnect=on";
        if (TheFields.HasField("donotcall"))    thestatsurl += "&donotcall=on";
        if (TheFields.HasField("invalid"))      thestatsurl += "&invalid=on";
        if (TheFields.HasField("nosalefalse"))  thestatsurl += "&nosalefalse=on";
        if (TheFields.HasField("nosaletrue"))   thestatsurl += "&nosaletrue=on";
        if (TheFields.HasField("sale"))         thestatsurl += "&sale=on";
        if (TheFields.HasField("used"))         thestatsurl += "&used=on";
    
	std::cout << "<br><br>\n";

	thestatsurl += "&mon1=" + TheFields.GetField("mon1");
	thestatsurl += "&day1=" + TheFields.GetField("day1");
	thestatsurl += "&year1=" + TheFields.GetField("year1");

	thestatsurl += "&mon2=" + TheFields.GetField("mon2");
	thestatsurl += "&day2=" + TheFields.GetField("day2");
	thestatsurl += "&year2=" + TheFields.GetField("year2");

        std::cout << "<meta http-equiv=\"refresh\" content=\"1;url=" << thestatsurl << "\">\n\n";

	std::cout << "<a href=\"" << thestatsurl << "\">Click here</a> if you are not redirected." << std::endl;

	std::cout << "<br><br>\n";

	}

void doImportData(Post TheFields) {

        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
        printHead("Import for: " + queue);
        
//        std::cout << "<br><form method=\"post\" enctype=\"multipart/form-data\" action=\"/cgi-bin/up.cgi\">" << std::endl;
        std::cout << "<br>Field List*: (separate with commas)" << std::endl;
        std::cout << "<br><input type=text name=\"fieldlist\" size=100 value=\"phone,title,name,address,city,state,zip,oc,paidamt,paiddate,comments,countynum\">" << std::endl;
        std::cout << "<br>* You MUST have a field named \"phone\"" << std::endl;
        std::cout << "<br>* \"phone\" field MUST be first" << std::endl;
        std::cout << "<br><br>" << std::endl;
        std::cout << "<br>Type List*: (separate with commas)" << std::endl;
        std::cout << "<br><input type=text name=\"typelist\" size=100 value=\"varchar(15),varchar(5),varchar(40),varchar(40),varchar(20),varchar(15),varchar(10),varchar(40),varchar(10),varchar(10),varchar(60),varchar(5)\">" << std::endl;
        std::cout << "<br>* The phone field should be a varchar" << std::endl;
        std::cout << "<br><br>" << std::endl;
        std::cout << printAlert("[?]","See README.import for more information on file layout.");
        std::cout << "Csv File:" << std::endl;
        std::cout << "<br><input type=file name=\"filefield\"><br><br>" << std::endl;


        std::cout << "<font size=-1 color=\"red\" face=\"helvetica,arial,sans\">";
        std::cout << "<br>Unless you know what you are doing do not change these.";
	std::cout << "<br>(make your data conform, because there are things in gnudialer";
        std::cout << "<br>that are hardcoded with some of these fieldnames and placement)<br><br>";
        std::cout << "</font>";
 
        std::cout << "<input type=hidden name=\"campaign\" value=\"" + queue + "\">";
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";

        std::cout << printSubmitConfirm("Import Records");
        std::cout << "</form><br><br>";
                
	std::cout << "<form method=post action=\"/cgi-bin/dialeradmin.cgi\">\n";
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
                
        }






void doReloadData(Post TheFields) {

        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
        printHead("Reload Data for: <br><br>" + queue);
        std::cout << "<br><br>" << std::endl;
  
        std::cout << printSubmitConfirm("Campaign Reload");
        std::cout << "<br>";
	}

void doDataReload(Post TheFields) {

        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";

	Queue TheQueue;
	u_long serverId = 1;
	TheQueue.ParseQueue(queue,serverId);

	MYSQL *mysql = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	unsigned int counter = 0;
	
	mysql = mysql_init(NULL);

	if(mysql == NULL) {
        	std::cerr << "MySql init failed!" << std::endl;
	}

	if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
        	                getMySqlUser().c_str(), \
                	        getMySqlPass().c_str(), \
                        	getDbName().c_str(), \
                        	3306, NULL, 0)) {

        	std::cerr << "MySql connection failed!" << std::endl;
        	}

	std::string query = "SELECT SQL_CALC_FOUND_ROWS phone,title,name,address,city,state,zip,oc,paidamt,paiddate,comments,countynum ";
	query += "FROM " + queue + " WHERE ((disposition > -7 AND disposition < 7) ";
	query += "OR disposition = 9 ";
	query += "OR disposition = 10 ";
	query += "OR disposition = 11) ";
	query += "INTO OUTFILE '/tmp/reload-" + queue + ".txt' FIELDS TERMINATED BY ',' ";
	query += "OPTIONALLY ENCLOSED BY '\"' LINES TERMINATED BY '\\r\\n'";

        if(mysql_query(mysql, query.c_str()) != 0) {
        	std::cout << "<pre>Error selecting reload records !</pre>" << std::endl;
	}
	
	query = "SELECT FOUND_ROWS()";
	        
        if(mysql_query(mysql, query.c_str()) != 0) {
		std::cout << "<pre>Error selecting found records !</pre>" << std::endl;
	} else {
                                
	        result = mysql_use_result(mysql);

        	row = mysql_fetch_row(result);
	
		counter = stoi(row[0]);
 
		std::cout << "</pre>";

        	if(mysql_errno(mysql)) {
        		std::cout << "<pre>Error fetching Sales from mysql!</pre>" << std::endl;
		}

        	if (!counter) {
        		std::cout << "<pre>No Records Reloaded!</pre>" << std::endl;
		} else {
			std::cout << "<pre>Reloaded " << counter << " records to " << "/tmp/reload-" << queue << ".txt</pre>";
			//std::system(("cp /tmp/reload-" + queue + ".txt /var/www/html/gnudialer/reload-" + queue + ".txt").c_str());
		}

        	mysql_free_result(result);
	}
}


	
void doExportSales(Post TheFields) {

        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
        printHead("Export Sales for: " + queue);
        std::cout << "<br><br>" << std::endl;
        printSubHead("Date Range");
        std::cout << "<br><br>" << std::endl;
        printDateFields(1);
        std::cout << "<br><br>To<br><br>\n";
        printDateFields(2);
        std::cout << "<br><br>" << std::endl;
  
        std::cout << printSubmit("Export Records");
        std::cout << "<br><br>";
	}

void doExportRecords(Post TheFields) {

        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
        std::string mon1 = TheFields.GetField("mon1");
        std::string day1 = TheFields.GetField("day1");
        std::string year1 = TheFields.GetField("year1");
        std::string date1 = "'" + year1 + "-" + mon1 + "-" + day1 + "'";

        std::string mon2 = TheFields.GetField("mon2");
        std::string day2 = TheFields.GetField("day2");
        std::string year2 = TheFields.GetField("year2");
        std::string date2 = "'" + year2 + "-" + mon2 + "-" + day2 + "'";
        
	Queue TheQueue;
	u_long serverId = 1;
	TheQueue.ParseQueue(queue, serverId);

	bool usecloser = TheQueue.GetSetting("usecloser").GetBool();

	MYSQL *mysql = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	
	unsigned int counter = 0;
	
	std::cout << "<pre>Campaign: " << queue << "</pre>" << std::endl;
	                
	mysql = mysql_init(NULL);

	if(mysql == NULL) {
        	std::cerr << "MySql init failed!" << std::endl;
	}

	if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
        	                getMySqlUser().c_str(), \
                	        getMySqlPass().c_str(), \
                        	getDbName().c_str(), \
                        	3306, NULL, 0)) {

        	std::cerr << "MySql connection failed!" << std::endl;
	}
        
        std::string query = "SELECT * FROM " + queue + " WHERE ";
	query += "(left(lastupdated,10) >= STR_TO_DATE(" + date1 + ", '%Y-%m-%d') ";
	query += "AND left(lastupdated,10) <= STR_TO_DATE(" + date2 + ", '%Y-%m-%d')) ";
	if (usecloser) {
		query += "AND closerdispo='12' AND subdispo < 'H5'";
	} else {
		query += "AND disposition='12'";
	}
        if(mysql_query(mysql, query.c_str()) != 0) {
        	std::cout << "<pre>Error selecting sales from mysql!</pre>" << std::endl;
	} else {
	        result = mysql_use_result(mysql);

		createExportFile(queue);
		std::cout << "<pre>";
		std::string theDB = "15";
	                
		if (queue.find("TAP",0) != std::string::npos) {
			theDB = "95";
		}	
		
		for(counter = 0; row = mysql_fetch_row(result); counter++) {
			std::string theDisplayRecord = "DB: ";
                                theDisplayRecord += rfill(theDB,3);		 		
				theDisplayRecord += rfill(std::string(row[0]),6);
				theDisplayRecord += " ";
                                theDisplayRecord += rfill(std::string(row[1]),10);
                                //theDisplayRecord += rfill(std::string(row[18]),3);
                                theDisplayRecord += rfill(std::string(row[16]),3);
                                theDisplayRecord += rfill(std::string(row[22]),3);
			std::cout << "<BR>" << theDisplayRecord;

//        OutFile << "  leadid : "  << std::string(row[0]) << std::endl;
//        OutFile << "   phone : "  << std::string(row[1]) << std::endl;
//        OutFile << "   title : "  << std::string(row[2]) << std::endl;
//        OutFile << "    name : "  << std::string(row[3]) << std::endl;
//        OutFile << " address : "  << std::string(row[4]) << std::endl;
//        OutFile << "    city : "  << std::string(row[5]) << std::endl;
//        OutFile << "   state : "  << std::string(row[6]) << std::endl;
//        OutFile << "     zip : "  << std::string(row[7]) << std::endl;
//        OutFile << "      oc : "  << std::string(row[8]) << std::endl;
//        OutFile << " paidamt : "  << std::string(row[9]) << std::endl;
//        OutFile << "paiddate : "  << std::string(row[10]) << std::endl;
//        OutFile << "comments : "  << std::string(row[11]) << std::endl;
//        OutFile << "  county : "  << std::string(row[12]) << std::endl;
//        OutFile << "     tzl : "  << std::string(row[13]) << std::endl;
//        OutFile << "     tzh : "  << std::string(row[14]) << std::endl;
//        OutFile << "attempts : "  << std::string(row[15]) << std::endl;
//        OutFile << " pickups : "  << std::string(row[16]) << std::endl;
//        OutFile << "abandons : "  << std::string(row[17]) << std::endl;
//        OutFile << " A dispo : "  << std::string(row[18]) << std::endl;
//        OutFile << "   agent : "  << std::string(row[19]) << std::endl;
//        OutFile << " C dispo : "  << std::string(row[20]) << std::endl;
//        OutFile << "  closer : "  << std::string(row[21]) << std::endl;
//        OutFile << "subdispo : "  << std::string(row[22]) << std::endl;
//        OutFile << "datetime : "  << std::string(row[23]) << std::endl;
//        OutFile << " cb_date : "  << std::string(row[24]) << std::endl;

                 	std::string theExportRecord = "DB: ";
                                theExportRecord += rfill(theDB,2);
                                theExportRecord += " ";
                                theExportRecord += rfill(std::string(row[0]),6);
                                theExportRecord += " "; 
                                theExportRecord += rfill(std::string(row[21]),3);
                                theExportRecord += rfill(std::string(row[19]),3);
                                theExportRecord += lfill(std::string(row[22]),2);
                                theExportRecord += "  ";
                                theExportRecord += rfill(std::string(row[1]),10);
                                theExportRecord += rfill(std::string(row[3]),40);
                                theExportRecord += rfill(std::string(row[4]),40);
                                theExportRecord += rfill(std::string(row[5]),20);
                                theExportRecord += rfill(std::string(row[6]),2);
                                theExportRecord += rfill(std::string(row[7]),10);
                                theExportRecord += rfill(std::string(row[23]),25);
                                theExportRecord += rfill(std::string(row[2]),4);                                
				theExportRecord += char(192);
				//theExportRecord += char(15);                

			writeExportRecord(queue,theExportRecord);
		}
	 
		std::cout << "</pre>";

        	if(mysql_errno(mysql)) {
        		std::cout << "<pre>Error fetching Sales from mysql!</pre>" << std::endl;
		}

        	if (!counter) {
        		std::cout << "<pre>No Sales exported!</pre>" << std::endl;
		} else {
			std::cout << "<pre>Exported " << counter << " records to " << "/tmp/exports/" << queue << "-CCYYMMDD.txt<br>";
			std::cout << "(CCYYMMDD means that todays date in that format is added)</pre>";
			//std::system(("cp /tmp/export-" + queue + ".txt /var/www/html/gnudialer/export-" + queue + ".txt").c_str());
		}

        	mysql_free_result(result);
	}
//
        if (usecloser) {
        	std::cout << "<hr>";
	        query = "SELECT * FROM " + queue + " WHERE ";
		//query += "(left(lastupdated,10) >= STR_TO_DATE(" + date1 + ", '%Y-%m-%d') ";
		//query += "AND left(lastupdated,10) <= STR_TO_DATE(" + date2 + ", '%Y-%m-%d')) ";

		query += "disposition = '12' AND closerdispo='0' ";

        	if(mysql_query(mysql, query.c_str()) != 0) {
        		std::cout << "<pre>Error selecting non-verified sales from mysql!</pre>" << std::endl;
		} else {
	        	result = mysql_use_result(mysql);
                                        
			//std::cout << "<pre>";

			//int sale_count = 0;
			for(counter = 0; row = mysql_fetch_row(result); counter++) {
				std::string theDisplayRecord = "";		 		
				theDisplayRecord += rfill(std::string(row[0]),6);
				theDisplayRecord += " ";
				theDisplayRecord += rfill(std::string(row[1]),10);
                                theDisplayRecord += " ";
				theDisplayRecord += rfill(std::string(row[3]),30);
                                theDisplayRecord += " ";
				theDisplayRecord += rfill(std::string(row[18]),3);
                                theDisplayRecord += " ";
				theDisplayRecord += rfill(std::string(row[19]),3);
                                theDisplayRecord += " ";
				theDisplayRecord += rfill(std::string(row[22]),3);
                                theDisplayRecord += " ";
				theDisplayRecord += rfill(std::string(row[23]),10);
				                                
                 		std::cout << "<BR>" << theDisplayRecord;

	        		if(mysql_errno(mysql)) {
        				std::cout << "<pre>Error fetching Sales from mysql!</pre>" << std::endl;
				}
				
				writeEODNonSale("EOD Non-Verified Sale",queue,std::string(row[0]));
			}

			if (!counter) {
				std::cout << "<pre>No Non-Verified Sales found!</pre>" << std::endl;
			} else {
				std::cout << "<pre>Printed " << counter << " non-verified records.</pre>";
			}
			
			mysql_free_result(result);
			std::cout << "</pre>";                                
		}
	}
}

void doCallistBuild(Post TheFields) {

        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        printHead("Build Lists for: " + queue);
        std::cout << "<br><br>" << std::endl;
        std::cout << "<input type=hidden name=\"queue\" value=\"" + queue + "\">";
        std::cout << printSubmit("Build Callists");
        std::cout << "<br><br>";
}

void doBuildCallists(Post TheFields) {

        std::string queue = TheFields.GetField("queue");
        std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
        
        MYSQL *mysql = NULL;
        MYSQL_RES *result;
        MYSQL_ROW row;
        mysql = mysql_init(NULL);

        if(mysql == NULL) {
                std::cerr << "MySql init failed!" << std::endl;
	}
        if(!mysql_real_connect(mysql, getMySqlHost().c_str(), \
                                getMySqlUser().c_str(), \
                                getMySqlPass().c_str(), \
                                getDbName().c_str(), \
                                3306, NULL, 0)) {
		std::cerr << "MySql connection failed!" << std::endl;
	}

        std::string query = "SELECT * FROM " + queue + " WHERE ";
        query += "disposition >=-5 AND disposition <= 5 AND ";
        query += "attempts <= 6 LIMIT 10000";
  
        if(mysql_query(mysql, query.c_str()) != 0) {
                std::cout << "<pre>Error selecting sales from mysql!</pre>" << std::endl;
	} else {
	        result = mysql_use_result(mysql);

        	createCallistFile(queue);

        	std::cout << "<pre>";
        
        	unsigned int counter = 0; 
        	for(counter = 0; row = mysql_fetch_row(result); counter++) {
	        	char myrecord[50];
        		std::string theCallistRecord;
        		sprintf(myrecord, "%8s%10s%3s%3s%20s%2s", row[0],row[1],row[10],row[11],row[5],row[6]);
        		theCallistRecord = std::string(myrecord);
                	std::cout << "<BR>" << theCallistRecord;
                	writeCallistRecord(queue,theCallistRecord);
		}

        	std::cout << "</pre>";

	        if(mysql_errno(mysql)) {
        	        std::cout << "<pre>Error fetching Sales from mysql!</pre>" << std::endl;
		}
        	if (!counter) {
                	std::cout << "<pre>No Records for Callist!</pre>" << std::endl;
		} else {
			std::cout << "<pre>Callist contains " << counter << " records.</pre>";
		}
        	mysql_free_result(result);
	}
}

int main() {

Post TheFields("text/html");

std::string mode = TheFields.GetField("mode");
std::cout << "Content-type: text/html\n" << std::endl;
std::cout << "<html><head><title></title>\n" << std::endl;

std::cout << "</head>\n" << std::endl;
std::cout << "<body background=\"/gnudialer/checkered.gif\">\n";

if (mode == "Import Data") {
	std::cout << "<form method=post enctype=\"multipart/form-data\" action=\"/cgi-bin/up.cgi\">\n";
        
} else {
	std::cout << "<form method=post action=\"/cgi-bin/dialeradmin.cgi\">\n";
	        
}

std::cout << "<font face=\"helvetica,arial,sans\">\n";
std::cout << "<center><table border=0 cellpadding=1 cellspacing=0>\n";
std::cout << "<tr><td bgcolor=\"#000000\">\n";
std::cout << "<center><table border=0 cellpadding=20 cellspacing=0>\n";
std::cout << "<tr><td bgcolor=\"#ffffff\">\n";

TheFields.LogHiddenQueryString();

// Enter all the modes here so we can easily
// deal with them in the "do" functions.

if (dialerIsRunning()) {

	if (mode == "login") {    
		doLogin(TheFields);
		mode = "Main Menu";
	}
	else if (mode == "Edit Script") doEditScript(TheFields);
	else if (mode == "Save Script") doSaveScript(TheFields);
	else if (mode == "Export Sales") doExportSales(TheFields);
        else if (mode == "Export Records") doExportRecords(TheFields);
	else if (mode == "Stop Campaign") doStopQueue(TheFields);
	else if (mode == "Manage Campaign") doManageCampaign(TheFields);
	else if (mode == "Agents Times") doShowAgentTime();
	else if (mode == "Show Stats") doShowStats(TheFields);
        else if (mode == "Show System Stats") doShowSysStats(TheFields);
        else if (mode == "Campaign Stats") doShowCampaignStats(TheFields);
        else if (mode == "System Stats") doShowSystemStats(TheFields);
	else if (mode == "All Campaign Stats") doShowAllCampaignStats(TheFields);
	else if (mode == "Agent Stats") doShowAgentStats(TheFields);
	else if (mode == "Logout") doLogout();
	else if (mode == "Main Menu") showMenu(TheFields);
	else if (mode == "Edit Campaign Members") showEditQueueMembers(TheFields);
	else if (mode == "Remove Campaign") doRemoveQueue(TheFields);
	else if (mode == "Add") doAdd(TheFields);
	else if (mode == "Remove") doRemove(TheFields);
	else if (mode == "Create Campaign") doCreateQueue(TheFields);
        else if (mode == "Import Data") doImportData(TheFields);
        else if (mode == "Create Reload Data") doReloadData(TheFields);
        else if (mode == "Campaign Reload") doDataReload(TheFields);
        else if (mode == "Print Record") doPrintRecord(TheFields);
        else if (mode == "Print Now") doPrintNow(TheFields);
        else if (mode == "Backup Data") doBackupData(TheFields);
        else if (mode == "Restore Data") doRestoreData(TheFields);
        else if (mode == "Backup Now") doBackupNow(TheFields);
        else if (mode == "Restore Now") doRestoreNow(TheFields);
        else if (mode == "Callist Build") doCallistBuild(TheFields);
        else if (mode == "Build Callists") doBuildCallists(TheFields); 
	else if (mode == "Finish Creating Campaign") doFinishCreatingQueue(TheFields);
        else if (mode == "Edit Campaign") doEditQueue(TheFields);
        else if (mode == "Edit Globals") doEditGlobals(TheFields); 
	else if (mode == "Start Campaign") doStartQueue(TheFields);
	else if (mode == "Create Agent") doCreateAgent();
        else if (mode == "Create Closer") doCreateCloser();
	else if (mode == "Finish Creating Agent") doFinishCreatingAgent(TheFields);
        else if (mode == "Finish Creating Closer") doFinishCreatingCloser(TheFields);
	else if (mode == "Delete Agent/Closer") doDeleteAgent(TheFields);
	else if (mode == "Edit Agent Info") doEditAgentInfo(TheFields);
	else if (mode == "Finish Editing Agent") doFinishEditingAgent(TheFields);
        else if (mode == "Save Campaign Settings") doFinishEditingCampaignSettings(TheFields);
        else if (mode == "Save Global Settings") doFinishEditingGlobals(TheFields);
        else if (mode == "Add Filter") doAddFilter(TheFields);
        else if (mode == "Del Filter") doDelFilter(TheFields);
        else if (mode == "Edit Filters") doEditFilters(TheFields);
        else if (mode == "Database Counts") doShowDBCounts(TheFields);
	else if (mode == "All DB Counts") doShowAllDBCounts(TheFields);
        else if (mode == "DB Counts Today") doShowAllDBCountsForToday(TheFields);
        else if (mode == "Save Filter Settings") doFinishEditingFilterSettings(TheFields);        
	else if (mode.empty()) std::cout << "Invalid Login.<br><br>\n";
	else std::cout << "Sorry! This option is not yet available.<br><br>" << mode << "<br><br>";

	if (!loginError) {
		if (mode != "Logout") {
			std::cout << "<br>";
                        if (mode == "Main Menu") {
				std::cout << printSubmit("Edit Globals");
                                std::cout << "&nbsp;&nbsp;&nbsp;";
				std::cout << printSubmit("All DB Counts");
				std::cout << printSubmit("DB Counts Today");
				std::cout << "&nbsp;&nbsp;&nbsp;";
				std::cout << "<br><br>";

			}
			if (mode != "Main Menu") {
				if (!mode.empty()) {
					std::cout << printSubmit("Main Menu");
		                        std::cout << "&nbsp;&nbsp;&nbsp;";
				}
			}
                        if (!mode.empty()) {
                        	std::cout << printSubmit("Logout");
                                std::cout << "&nbsp;&nbsp;&nbsp;";
                                
	                        if (mode == "Main Menu") {
					std::cout << "Help ";
					std::string theVersion = "Puff 18.1.09";
					std::cout << printAlert("[?]","Anytime you see this next to a Feature, \\njust click on it for general help information.\\n\\nIf you require install or custom programming\\n assistance, please goto \\n\\nhttp://gnudialer.org \\n\\nfor our contact information and hourly rates.\\n(or if you just feel like making a donation)\\n\\nOtherwise, please visit the user forum at\\n\\n http://developers.gnudialer.org/?task=forum \\n\\nfor free assistance.\\n\\n\\n\\nCopyright (c) 2005, GnuDialer Project\\n\\nHeath Schultz - heath1444(at)yahoo.com \\nJamerson Medley - nixtux2003(at)yahoo.com\\nRichard Lyman - richard(at)dynx.net\\n\\nThis program is free software, distributed under\\nthe terms ofthe GNU General Public License.\\n\\nVersion: " + theVersion + "\\n");
				}
			}                                                               
		}
	}

	std::cout << "<SCRIPT>";
	std::cout << "window.status  = \"This is the status as of now!!!\";";
	std::cout << "</SCRIPT>";
	std::cout << "</form>";
                               
} else {
	std::cout << "<br><br>GnuDialer not running!";
}

std::cout << "</td></tr>\n";
std::cout << "</table></center>\n";
std::cout << "</td></tr>\n";
std::cout << "</table></center>\n";

TheFields.PrintHiddenQueryString();

std::cout << "</font>\n</body></html>" << std::endl;

return 0;

}
