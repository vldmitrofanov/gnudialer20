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

//const std::string nbspx3() { return "&nbsp;&nbsp;&nbsp;"; }
//const std::string cell() { return "</td><td>"; }

int main() {

Post TheFields("text/html");

// std::string mode = TheFields.GetField("statsmode");

std::cout << "Content-type: text/html\n" << std::endl;

std::cout << "<html><head><title><body bgcolor=\"#ffffff\" text=\"#000000\">\n";

startMon = atoi(TheFields.GetField("mon1").c_str());
startDay = atoi(TheFields.GetField("day1").c_str());
startYear = atoi(TheFields.GetField("year1").c_str());

endMon = atoi(TheFields.GetField("mon2").c_str());
endDay = atoi(TheFields.GetField("day2").c_str());
endYear = atoi(TheFields.GetField("year2").c_str());

//int size = 0;

unsigned long int tempTimeLogged = 0, accumulator = 0;

std::string temp;
AgentLoginHelperList TheAgents;
TheAgents.Parse();
TheAgents.sort();

std::cout << "Total Logtime for All Agents: ";
std::cout << startMon << "-" << startDay << "-" << startYear;
std::cout << " thru " << endMon << "-" << endDay << "-" << endYear;
std::cout << "<br><br>\n";
std::cout << std::endl << std::endl;

for (int i = 0; i < TheAgents.size(); i++) {

	if (TheAgents.at(i).GetTimeLogged()) {

		tempTimeLogged = TheAgents.at(i).GetTimeLogged();

		accumulator += tempTimeLogged;

		std::cout << "Agent ";
		std::cout << TheAgents.at(i).GetNumber() << ": &nbsp;&nbsp;&nbsp;" << std::endl;
		std::cout << formatHours(tempTimeLogged) << "<br>\n" << std::endl;

		}

	}

std::cout << "<br>";
std::cout << "Total: " << formatHours(accumulator);

std::cout << "</body></html>\n";

return 0;

}
