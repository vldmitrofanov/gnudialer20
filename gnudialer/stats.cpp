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

#include "stats.h"
#include "post.h"

const std::string nbspx3() { return "&nbsp;&nbsp;&nbsp;"; }
const std::string cell() { return "</td><td>"; }

int main() {

	excludeClosers = false;

	Post TheFields("text/html");

	std::string mode = TheFields.GetField("statsmode");
	std::string selection = TheFields.GetField("selection");
        std::string queue = TheFields.GetField("queue");
        
	std::cout << "Content-type: text/html\n" << std::endl;

	std::cout << "<html><head>\n";

	//this was some code i was playing with to set landscape mode ages ago
	//std::cout << "<style type=\"text/css\" media=\"print\">\n";
	//std::cout << "div.page	{ \n";
	//std::cout << "writing-mode: tb-rl;\n";
	//std::cout << "height: 80%;\n";
	//std::cout << "margin: 10% 0%;\n";
	//std::cout << "}\n";
	//std::cout << "</style>\n";
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

		std::cout << selection << ": ";
		std::cout << startMon << "-" << startDay << "-" << startYear;
		std::cout << " thru " << endMon << "-" << endDay << "-" << endYear;

		std::cout << "<br><br>\n";
	}

	std::cout << "<table border=0 cellpadding=3 cellspacing=0><tr><td>&nbsp;" << std::endl;

	std::cout << cell() << "Calls" << nbspx3() << std::endl;
	//std::cout << cell() << "Sales" << nbspx3() << std::endl;
	//std::cout << cell() << "No Sales" << nbspx3() << std::endl;

	if (TheFields.HasField("callback"))           std::cout << cell() << "CB"       	<< nbspx3() << std::endl;
	if (TheFields.HasField("noanswer"))           std::cout << cell() << "No Ans"      	<< nbspx3() << std::endl;
	if (TheFields.HasField("voicemail"))          std::cout << cell() << "VM"      		<< nbspx3() << std::endl;
	if (TheFields.HasField("busy"))               std::cout << cell() << "Busies"          	<< nbspx3() << std::endl;
	if (TheFields.HasField("fastbusy"))           std::cout << cell() << "Fast Busies"     	<< nbspx3() << std::endl;
	if (TheFields.HasField("fax"))                std::cout << cell() << "Fax Mach"    	<< nbspx3() << std::endl;
	if (TheFields.HasField("disconnect"))         std::cout << cell() << "Disc"     	<< nbspx3() << std::endl;
	if (TheFields.HasField("donotcall"))          std::cout << cell() << "DNC"    		<< nbspx3() << std::endl;
	if (TheFields.HasField("invalid"))            std::cout << cell() << "Inval"        	<< nbspx3() << std::endl;
	if (TheFields.HasField("nosalefalse"))        std::cout << cell() << "Other"    	<< nbspx3() << std::endl;
	if (TheFields.HasField("nosaletrue"))         std::cout << cell() << "NoSale"     	<< nbspx3() << std::endl;
	if (TheFields.HasField("sale"))               std::cout << cell() << "Sale"            	<< nbspx3() << std::endl;
	if (TheFields.HasField("used"))               std::cout << cell() << "Used"            	<< nbspx3() << std::endl;

	std::cout << cell() << "Cntcts" << nbspx3() << std::endl;
	std::cout << cell() << "TT" << nbspx3() << std::endl;
	std::cout << cell() << "Conv&nbsp;\%" << nbspx3() << std::endl;
	std::cout << cell() << "Cntcts/Hr." << nbspx3() << std::endl;
	std::cout << cell() << "Sales/Hr." << nbspx3() << std::endl;

	std::cout << "</td></tr><tr><td>" << std::endl;

	if (mode == "agent") {
        	std::cout << "Agent ";
		//std::cout << "Agent&nbsp;";
	} else {
		excludeClosers = true;
	}

	std::cout << selection << nbspx3() << std::endl;
	std::cout << cell() << TheStats.TotalCalls() << std::endl;
	//std::cout << cell() << TheStats.GetSales() << std::endl;
	//std::cout << cell() << TheStats.GetNoSales() << std::endl;

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
	std::cout << cell() << formatDouble(TheStats.GetConversion()) << "\%" << std::endl;
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
			//std::cout << cell() << TheStats.GetSales(temp) << std::endl;
			//std::cout << cell() << TheStats.GetNoSales(temp) << std::endl;

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
			std::cout << cell() << formatDouble(TheStats.GetConversion(temp)) << "\%" << std::endl;
			std::cout << cell() << formatDouble(TheStats.GetContactsPerHour(temp)) << std::endl;
			std::cout << cell() << formatDouble(TheStats.GetSalesPerHour(temp)) << std::endl;
		}
	}

	std::cout << "</td></tr></table>\n";
	
	std::cout << "<br><form method=post action=\"/cgi-bin/dialeradmin.cgi\">\n";
	std::cout << "<input type=hidden name=\"queue\" value=\"" << selection << "\">";
	std::cout << "<input type=submit name=\"mode\" value=\"Main Menu\">";
        std::cout << "</form>";
        
	std::cout << "</body></html>\n";
	return 0;
}

