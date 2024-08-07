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

#include "newstats.h"
#include "stats.h"
#include "post.h"


int main() {

	excludeClosers = false;

	Post TheFields("text/html");
	std::cout << "Content-type: text/html\n" << std::endl;

	std::cout << "<html><head><title>Statistics</title></head>\n";
	std::cout << "<body bgcolor=#ffffff>\n\n";

	std::string mode = TheFields.GetField("statsmode");
	std::string selection = TheFields.GetField("selection");
        std::string queue = TheFields.GetField("queue");
        
	std::cout << "<title></title><body bgcolor=\"#ffffff\" text=\"#000000\">\n";

	startMon 	= atoi(TheFields.GetField("mon1").c_str());
	startDay 	= atoi(TheFields.GetField("day1").c_str());
	startYear 	= atoi(TheFields.GetField("year1").c_str());
	endMon 		= atoi(TheFields.GetField("mon2").c_str());
	endDay 		= atoi(TheFields.GetField("day2").c_str());
	endYear 	= atoi(TheFields.GetField("year2").c_str());

	double totalTalkTime = 0, totalLoggedHours = 0;
	int totalUsed = 0;

	int size = 0;
	std::string temp;
	QueueList TheQueues;
	AgentList TheAgents;
	AgentLoginHelperList TheAgentLogins;

	Composite * TheComposite;

	if (mode == "campaign") {
		TheComposite = new CampaignComposite;
		TheAgents.ParseAgentList();
		size = TheAgents.size();
		TheComposite->SetName(selection);
		TheComposite->Parse(TheAgents);
	}

	if (mode == "agent") {
		TheComposite = new AgentComposite;
		TheQueues.ParseQueues();
		size = TheQueues.size();
		TheComposite->SetNumber(selection);
		TheComposite->Parse(TheQueues);
		TheAgentLogins.Parse();
	}
        
	TheQueues.ParseQueues();
                
        for (int i = 0; i < TheQueues.size(); i++) {
                std::string tempCampaign = TheQueues.at(i).GetName();
                TheComposite->SetName(selection);
                TheComposite->Parse(TheAgents);                
        
        }
        
	Composite & TheStats = (*TheComposite);
	
	if (mode == "agent") {
		std::cout << "Agent " << selection << ": ";
		//std::cout << "Agent&nbsp;" << selection << ": ";
		std::cout << startMon << "-" << startDay << "-" << startYear;
		std::cout << " thru " << endMon << "-" << endDay << "-" << endYear;

		std::cout << "<br><br>\n";

		totalTalkTime = static_cast<double>(TheStats.GetTalkTime());
		totalLoggedHours = static_cast<double>(TheAgentLogins.where(atoi(selection.c_str())).GetTimeLogged());
		std::cout << "Logged Hours: " << formatHours(TheAgentLogins.where(atoi(selection.c_str())).GetTimeLogged());

		if (totalLoggedHours) {
			std::cout << "<br><br>Talk Percent: " << formatDouble((totalTalkTime / totalLoggedHours) * 100.0) << "\%";
		} else {
			std::cout << "<br><br>Talk Percent: 0.00\%";
		}

		std::cout << "<br><br>\n";
	}

	if (mode == "campaign") {

                //std::cout << selection << ": ";
		std::cout << startMon << "-" << startDay << "-" << startYear;
		std::cout << " thru " << endMon << "-" << endDay << "-" << endYear;

		std::cout << "<br><br>\n";
	}

//start table
        		std::cout << "<table border=0 cellpadding=3 cellspacing=0>";

//start table label info        		


        		//std::cout << "<tr><td>" << selection << "&nbsp;&nbsp;&nbsp;&nbsp;" << std::endl;
                        std::cout << "<tr><td>------------&nbsp;&nbsp;&nbsp;&nbsp;" << std::endl;
                        
        		std::cout << cell() << "Calls" << nbspx3() << std::endl;

		        if (TheFields.HasField("callback"))           std::cout << cell() << "Abn /CB"          << nbspx3() << std::endl;
        		if (TheFields.HasField("noanswer"))           std::cout << cell() << "No Ans"           << nbspx3() << std::endl;
        		if (TheFields.HasField("voicemail"))          std::cout << cell() << "VM"               << nbspx3() << std::endl;
        		if (TheFields.HasField("busy"))               std::cout << cell() << "Busy"           	<< nbspx3() << std::endl;
        		if (TheFields.HasField("fastbusy"))           std::cout << cell() << "Cong"      	<< nbspx3() << std::endl;
        		if (TheFields.HasField("fax"))                std::cout << cell() << "Fax"              << nbspx3() << std::endl;
        		if (TheFields.HasField("disconnect"))         std::cout << cell() << "Disc"             << nbspx3() << std::endl;
        		if (TheFields.HasField("donotcall"))          std::cout << cell() << "DNC"              << nbspx3() << std::endl;
        		if (TheFields.HasField("invalid"))            std::cout << cell() << "Inval"            << nbspx3() << std::endl;
        		if (TheFields.HasField("nosalefalse"))        std::cout << cell() << "Other"            << nbspx3() << std::endl;
        		if (TheFields.HasField("nosaletrue"))         std::cout << cell() << "No Sale"          << nbspx3() << std::endl;
        		if (TheFields.HasField("sale"))               std::cout << cell() << "Sale"             << nbspx3() << std::endl;
        		if (TheFields.HasField("used"))               std::cout << cell() << "Used"             << nbspx3() << std::endl;

        		std::cout << cell() << "Cnts" << nbspx3() << std::endl;
        		std::cout << cell() << "TT" << nbspx3() << std::endl;
        		std::cout << cell() << "Abn/ Cnv\%" << nbspx3() << std::endl;
        		std::cout << cell() << "Cnts /Hr." << nbspx3() << std::endl;
        		std::cout << cell() << "Sales /Hr." << nbspx3() << std::endl;

      
        		std::cout << "</td></tr><tr>" << std::endl;

//load machine stats for a campaign
        for (int i = 0; i < TheQueues.size(); i++) {
		std::string tempCampaign = TheQueues.at(i).GetName();
		if (tempCampaign != "CLOSER" && tempCampaign != "DNC") {
			SysDayStats::LoadDateRange(tempCampaign,startMon,startDay,startYear,endMon,endDay,endYear);
		}
	}                                                                
	//SysDayStats::LoadDateRange(selection,startMon,startDay,startYear,endMon,endDay,endYear);

//start print machine stats
	if (SysDayStats::RangeMakesSense() && mode == "campaign") {
		if (SysDayStats::Size()) {

        		std::cout << cell() << " &nbsp;&nbsp;system&nbsp; "         << nbspx3() << std::endl;

        		std::cout << cell() << SysDayStats::Calls()      << std::endl;
        		if (TheFields.HasField("callback"))           std::cout << cell() << SysDayStats::Abandons()      << std::endl;
        		if (TheFields.HasField("noanswer"))           std::cout << cell() << SysDayStats::NoAnswers()      << std::endl;
        		if (TheFields.HasField("voicemail"))          std::cout << cell() << SysDayStats::AnsMachs()      << std::endl;
        		if (TheFields.HasField("busy"))               std::cout << cell() << SysDayStats::Busies()      << std::endl;
        		if (TheFields.HasField("fastbusy"))           std::cout << cell() << SysDayStats::Congestions()      << std::endl;
        		if (TheFields.HasField("fax"))                std::cout << cell() << "0"      << std::endl;
			if (TheFields.HasField("disconnect"))         std::cout << cell() << SysDayStats::Disconnects()      << std::endl;	
                        if (TheFields.HasField("donotcall"))          std::cout << cell() << ""      << std::endl;
			if (TheFields.HasField("invalid"))            std::cout << cell() << ""      << std::endl;
			if (TheFields.HasField("nosalefalse"))        std::cout << cell() << ""      << std::endl;
                        if (TheFields.HasField("nosaletrue"))         std::cout << cell() << ""      << std::endl;
                        if (TheFields.HasField("sale"))               std::cout << cell() << ""      << std::endl;
        		if (TheFields.HasField("used"))               std::cout << cell() << SysDayStats::Disconnects()      << std::endl;

                        std::cout << cell() << ""      << std::endl;
                        std::cout << cell() << ""      << std::endl;                                              
		        if (SysDayStats::Calls() > 0 && SysDayStats::Abandons() > 0) {
        			double d_abns  = static_cast<double>(SysDayStats::Abandons());
                		double d_calls = static_cast<double>(SysDayStats::Calls());
                		double abn_perc = d_abns / d_calls * 100.00;
        			std::cout << cell() << std::setiosflags(std::ios_base::fixed) << std::setprecision(2) << abn_perc << std::endl;
			} else {
				std::cout << cell() << "0"      << std::endl;
			}                
  
                        std::cout << cell() << ""      << std::endl;
                        std::cout << cell() << ""      << std::endl;
                      
        		//std::cout << "</td></tr></table>\n";
                        std::cout << "</td></tr>";
                        
		} else {
			std::cout << "No data for date range!" << std::endl;
		}
	//}
	}

	std::cout << "</td></tr><tr><td>" << std::endl;
// end print machine stats

	if (mode == "agent") {
        	std::cout << "Agent ";
	} else {
		excludeClosers = true;
	}

// start print agent stats
	std::cout << " &nbsp;&nbsp;Agents&nbsp; " << nbspx3() << std::endl;
	std::cout << cell() << TheStats.TotalCalls() << std::endl;

	if (TheFields.HasField("callback"))           std::cout << cell() << TheStats.NumDispos("0")      << std::endl;
	if (TheFields.HasField("noanswer"))           std::cout << cell() << TheStats.NumDispos("2")      << std::endl;
	if (TheFields.HasField("voicemail"))          std::cout << cell() << TheStats.NumDispos("3")      << std::endl;
	if (TheFields.HasField("busy"))               std::cout << cell() << TheStats.NumDispos("4")      << std::endl;
	if (TheFields.HasField("fastbusy"))           std::cout << cell() << TheStats.NumDispos("5")      << std::endl;
	if (TheFields.HasField("fax"))                std::cout << cell() << TheStats.NumDispos("6")      << std::endl;
	if (TheFields.HasField("disconnect"))         std::cout << cell() << TheStats.NumDispos("7")      << std::endl;
	if (TheFields.HasField("donotcall"))          std::cout << cell() << TheStats.NumDispos("8")      << std::endl;
	if (TheFields.HasField("invalid"))            std::cout << cell() << TheStats.NumDispos("9")      << std::endl;
	if (TheFields.HasField("nosalefalse"))        std::cout << cell() << TheStats.NumDispos("10")     << std::endl;
	if (TheFields.HasField("nosaletrue"))         std::cout << cell() << TheStats.NumDispos("11")     << std::endl;
	if (TheFields.HasField("sale"))               std::cout << cell() << TheStats.NumDispos("12")     << std::endl;
	totalUsed = (TheStats.NumDispos("6") + TheStats.NumDispos("7") + TheStats.NumDispos("8") + TheStats.NumDispos("9") + TheStats.NumDispos("10") + TheStats.NumDispos("11") + TheStats.NumDispos("12"));
	if (TheFields.HasField("used"))               std::cout << cell() << totalUsed << std::endl;

	std::cout << cell() << TheStats.GetContacts() << std::endl;
	std::cout << cell() << formatHours(TheStats.GetTalkTime()) << std::endl;
	std::cout << cell() << formatDouble(TheStats.GetConversion()) << std::endl;
	std::cout << cell() << formatDouble(TheStats.GetContactsPerHour()) << std::endl;
	std::cout << cell() << formatDouble(TheStats.GetSalesPerHour()) << std::endl;

	excludeClosers = true; //was false

	for (int i = 0; i < size; i++) {
		if (mode == "campaign") {
			temp = itos(TheAgents.at(i).GetNumber());
		} else {
			temp = TheQueues.at(i).GetName();
		}

		if (TheStats.GetTalkTime(temp)) {
			std::cout << "</td></tr><tr><td>\n";
			if (mode == "campaign") std::cout << "Agent ";
			std::cout << temp << nbspx3() << std::endl;
			std::cout << cell() << TheStats.TotalCalls(temp) << std::endl;

			if (TheFields.HasField("callback"))           std::cout << cell() << TheStats.NumDispos("0",temp)      << std::endl;
			if (TheFields.HasField("noanswer"))           std::cout << cell() << TheStats.NumDispos("2",temp)      << std::endl;
			if (TheFields.HasField("voicemail"))          std::cout << cell() << TheStats.NumDispos("3",temp)      << std::endl;
			if (TheFields.HasField("busy"))               std::cout << cell() << TheStats.NumDispos("4",temp)      << std::endl;
			if (TheFields.HasField("fastbusy"))           std::cout << cell() << TheStats.NumDispos("5",temp)      << std::endl;
			if (TheFields.HasField("fax"))                std::cout << cell() << TheStats.NumDispos("6",temp)      << std::endl;
			if (TheFields.HasField("disconnect"))         std::cout << cell() << TheStats.NumDispos("7",temp)      << std::endl;
			if (TheFields.HasField("donotcall"))          std::cout << cell() << TheStats.NumDispos("8",temp)      << std::endl;
			if (TheFields.HasField("invalid"))            std::cout << cell() << TheStats.NumDispos("9",temp)      << std::endl;
			if (TheFields.HasField("nosalefalse"))        std::cout << cell() << TheStats.NumDispos("10",temp)     << std::endl;
			if (TheFields.HasField("nosaletrue"))         std::cout << cell() << TheStats.NumDispos("11",temp)     << std::endl;
			if (TheFields.HasField("sale"))               std::cout << cell() << TheStats.NumDispos("12",temp)     << std::endl;
			totalUsed = (TheStats.NumDispos("6",temp) + TheStats.NumDispos("7",temp) + TheStats.NumDispos("8",temp) + TheStats.NumDispos("9",temp) + TheStats.NumDispos("10",temp) + TheStats.NumDispos("11",temp) + TheStats.NumDispos("12",temp));
			if (TheFields.HasField("used"))               std::cout << cell() << totalUsed << std::endl;

			std::cout << cell() << TheStats.GetContacts(temp) << std::endl;
			std::cout << cell() << formatHours(TheStats.GetTalkTime(temp)) << std::endl;
			std::cout << cell() << formatDouble(TheStats.GetConversion(temp)) << std::endl;
			std::cout << cell() << formatDouble(TheStats.GetContactsPerHour(temp)) << std::endl;
			std::cout << cell() << formatDouble(TheStats.GetSalesPerHour(temp)) << std::endl;
		}
	}

	std::cout << "</td></tr>";
	
//end print agent stats

//end table and show main menu button
	std::cout << "</table>\n";
	
	std::cout << "<br><form method=post action=\"/cgi-bin/dialeradmin.cgi\">\n";
	std::cout << "<input type=hidden name=\"queue\" value=\"" << queue << "\">";
	std::cout << "<input type=submit name=\"mode\" value=\"Main Menu\">";
        std::cout << "</form>";
        
	std::cout << "</body></html>\n";
	return 0;
}




