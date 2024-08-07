/*
 * GnuDialer - Complete, free predictive dialer
 *
 * Complete, free predictive dialer for contact centers.
 *
 * Copyright (C) 2005, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <cstdlib>
#include "agent.h"
#include "queue.h"
#include "itos.h"

#ifndef STATS
#define STATS

bool startOk, endOk;

int startMon;
int startDay;
int startYear;

int endMon;
int endDay;
int endYear;

class AgentLoginHelper {

	public:

	AgentLoginHelper() { 
		loggedInTime = 0;
		runningTotal = 0;
	}
	~AgentLoginHelper() {}

	void SetNumber(const int & number) { herNumber = number; }
	const int & GetNumber()const { return herNumber; }

	void NoteLogin(const long int & thetime) {
		loggedInTime = thetime;
	}

	void NoteLogoff(const long int & thetime) {
		if (loggedInTime) {
			runningTotal += thetime - loggedInTime;
			loggedInTime = 0;
		}
	}

	const unsigned long int GetTimeLogged()const {
		timeval tv;
		if (loggedInTime) {
			//std::cout << "~~~" << tv.tv_sec << " - " << loggedInTime << "<br><br>\n";
			gettimeofday(&tv,NULL);
			return runningTotal + (tv.tv_sec - loggedInTime);
		} else {
			return runningTotal;
		}
	}

	private:

	unsigned long int loggedInTime, runningTotal;
	int herNumber;

};

const bool operator< (const AgentLoginHelper & lhs, const AgentLoginHelper & rhs) {
        if (lhs.GetNumber() < rhs.GetNumber()) {
                return true;
        } else {
                return false;
        }
}

class AgentLoginHelperList {

	public:

	AgentLoginHelperList() {}
	~AgentLoginHelperList() {}

	void handleEntry(const std::string & raw) {

		time_t thetime;
		tm * ptm;

		int mon, day, year;

		std::string time, agent, type;

		std::stringstream EntryStream;

		if (raw.find("Agent/",0) != std::string::npos) {

			EntryStream << raw;
			std::getline(EntryStream,time,'|');
			std::getline(EntryStream,agent,'|');
			std::getline(EntryStream,agent,'|');
			std::getline(EntryStream,agent,'|');
			std::getline(EntryStream,type,'|');

			agent = agent.substr(6,agent.length()-6);
			thetime = atoi(time.c_str());
			//time = time.substr(3,time.length()-3);
		}

		if (type == "AGENTLOGIN") {

			ptm = localtime(&thetime);
			mon = ptm->tm_mon + 1;
			day = ptm->tm_mday;
			year = ptm->tm_year + 1900;

	        	// These are globals because we want to squeeze every ounce of
	        	// efficiency as we can for the program.
			startOk = false;
			endOk = false;

			if (year > startYear) startOk = true;
			if (year == startYear) {
				if (mon > startMon) startOk = true;
				if (mon == startMon) {
					if (day >= startDay) startOk = true;
				}
			}

			if (year < endYear) endOk = true;
			if (year == endYear) {
				if (mon < endMon) endOk = true;
				if (mon == endMon) {
					if (day <= endDay) endOk = true;
				}
			}

	        	if (startOk && endOk) {
				where(atoi(agent.c_str())).NoteLogin(atoi(time.c_str()));
			}
		}

		if (type == "AGENTLOGOFF") {
			where(atoi(agent.c_str())).NoteLogoff(atoi(time.c_str()));
		}
	}

	void Parse() {

		std::ifstream AgentsIn;
		AgentsIn.open("/var/log/asterisk/queue_log");
		for (std::string tempLine; std::getline(AgentsIn,tempLine,'\n'); ) {
			handleEntry(tempLine);
		}
		AgentsIn.close();
	}
	
	void push_back(const int & number) {

		AgentLoginHelper TempAgentLoginHelper;
		TempAgentLoginHelper.SetNumber(number);
		TheAgentLoginHelpers.push_back(TempAgentLoginHelper);
	}

	void push_back_if_doesnt_exist(const int & number) {

		bool exists = false;
		for (unsigned int i = 0; i < TheAgentLoginHelpers.size(); i++) {
			if (TheAgentLoginHelpers.at(i).GetNumber() == number) {
				exists = true;
				break;
			}
		}

		if (!exists) {
			push_back(number);
		}
	}

	AgentLoginHelper & where(const int & number) {
		push_back_if_doesnt_exist(number);
		for (unsigned int i = 0; i < TheAgentLoginHelpers.size(); i++) {
			if (TheAgentLoginHelpers.at(i).GetNumber() == number) {
				return TheAgentLoginHelpers.at(i);
			}
		}

		throw(xLoopEnd("Exception thrown: Didn't find a loginhelper for agent " + number));
	}

	const int size() { return TheAgentLoginHelpers.size(); }
	const AgentLoginHelper at(int whichOne) { return TheAgentLoginHelpers.at(whichOne); }

	void sort() {
		std::sort(TheAgentLoginHelpers.begin(),TheAgentLoginHelpers.end());
	}

	private:

	std::vector<AgentLoginHelper> TheAgentLoginHelpers;

	bool countAll;

};

long int stoi(const std::string & theString) { return atoi(theString.c_str()); }

class CallEntry {

	public:

	CallEntry() {}
	~CallEntry() {}

	void Parse(const std::string & rawData) {

		std::string temp;
		std::stringstream CallStream, DateStream;
		CallStream << rawData;
		std::getline(CallStream,itsLeadId,',');
		std::getline(CallStream,itsTime,',');
		std::getline(CallStream,itsTalkTime,',');
		std::getline(CallStream,itsDateString,',');
		std::getline(CallStream,itsDisposition,',');

		// The disposition may not be available in the normal course of operation
		// since we send it separately via the CRM application.

		if (itsLeadId.empty() || \
	    		itsTime.empty() || \
	    		itsTalkTime.empty() || \
	    		itsDateString.empty()) {

			std::cerr << "Error parsing entry!" << std::endl;

		} else {

			DateStream << itsDateString;

			std::getline(DateStream,temp,'-');
			itsMonth = atoi(temp.c_str());

			std::getline(DateStream,temp,'-');
			itsDay = atoi(temp.c_str());

			std::getline(DateStream,temp,'-');
			itsYear = atoi(temp.c_str());

			if (itsYear < 2000) {
				itsYear += 1900;
			}

			if (itsDisposition.empty()) {
				itsDisposition = "0";
			}
		}
	}

	const std::string & GetTalkTime()const { return itsTalkTime; }
	const std::string & GetLeadId()const { return itsLeadId; }
	const std::string & GetTime()const { return itsTime; }
	const std::string & GetDateString()const { return itsDateString; }
	const std::string & GetDisposition()const { return itsDisposition; }
	const int & GetMonth()const { return itsMonth; }
	const int & GetDay()const { return itsDay; }
	const int & GetYear()const { return itsYear; }

	private:

	// 69,1123463672,63,8-7-105,0
	std::string itsLeadId, itsTime, itsTalkTime, itsDateString, itsDisposition;
	int itsMonth, itsDay, itsYear;

};

bool isInRange(const CallEntry & TheCallEntry) {

	// These are globals because we want to squeeze every ounce of
	// efficiency as we can for the program.
	startOk = false;
	endOk = false;

	if (TheCallEntry.GetYear() > startYear) startOk = true;
	if (TheCallEntry.GetYear() == startYear) {
		if (TheCallEntry.GetMonth() > startMon) startOk = true;
		if (TheCallEntry.GetMonth() == startMon) {
			if (TheCallEntry.GetDay() >= startDay) startOk = true;
		}
	}

        if (TheCallEntry.GetYear() < endYear) endOk = true;
        if (TheCallEntry.GetYear() == endYear) {
                if (TheCallEntry.GetMonth() < endMon) endOk = true;
                if (TheCallEntry.GetMonth() == endMon) {
                        if (TheCallEntry.GetDay() <= endDay) endOk = true;
                }
        }

	return (startOk && endOk);
}

class StatFile {

	public:

	StatFile() {}
	~StatFile() {}

	void SetAgent(const std::string & agent) { itsAgent = agent; }
	void SetCampaign(const std::string & campaign) { itsCampaign = campaign; }

	const std::string & GetAgent()const { return itsAgent; }
	const std::string & GetCampaign()const { return itsCampaign; }

	void push_back(const std::string & raw) {

		CallEntry TheCallEntry;
		TheCallEntry.Parse(raw);

		itsCallEntries.push_back(TheCallEntry);
	}

	const long int size()const { return itsCallEntries.size(); }
	const CallEntry & at(const long int & i)const { return itsCallEntries.at(i); }

	const long int size_in_range()const {

		long int counter = 0;

		for (unsigned int i = 0; i < itsCallEntries.size(); i++) {

			if (isInRange(itsCallEntries.at(i))) {
				counter++;
			}
		}
		return counter;
	}

	const long int GetTalkTime()const {

		long int accumulator = 0;

		for (unsigned int i = 0; i < itsCallEntries.size(); i++) {
			if (isInRange(itsCallEntries.at(i))) {
				accumulator += std::stoi(itsCallEntries.at(i).GetTalkTime());
			}
		}

		return accumulator;
	}

	const long int NumDispos(const std::string & dispo) {

		long int counter = 0;

		for (unsigned int i = 0; i < itsCallEntries.size(); i++) {

			if (isInRange(itsCallEntries.at(i))) {

				if (itsCallEntries.at(i).GetDisposition() == dispo) {
					counter++;
				}
			}
		}

		return counter;
	}

	private:

	std::vector<CallEntry> itsCallEntries;
	std::string itsAgent, itsCampaign;
};

StatFile GetStatFile(const std::string & agent, const std::string & campaign) {

	StatFile TheStatFile;
	std::ifstream StatFileIn;
	StatFileIn.open(static_cast<std::string>("/var/log/asterisk/gnudialer/" + agent + "-" + campaign).c_str());
	for (std::string tempLine; std::getline(StatFileIn,tempLine,'\n'); ) {
		TheStatFile.push_back(tempLine);
	}
	StatFileIn.close();

	return TheStatFile;
}

bool excludeClosers;

class Composite {

	public:
	Composite() {}
	virtual ~Composite() {}

	virtual void Parse(const AgentList &) {}
	virtual void Parse(QueueList &) {}

	virtual void SetName(const std::string &) {}
	virtual void SetNumber(const std::string &) {}

	virtual const int GetTalkTime(const std::string &)=0;
	virtual const int NumDispos(const std::string &, const std::string &)=0;
	virtual const int TotalCalls(const std::string &)=0;

	const int size()const { return TheStatFiles.size(); }
	const StatFile & at(int i)const { return TheStatFiles.at(i); }

	const int GetTalkTime() {

		int accumulator = 0;

		for (unsigned int i = 0; i < TheStatFiles.size(); i++) {
			if ((excludeClosers && atoi(TheStatFiles.at(i).GetAgent().c_str()) < 600) || excludeClosers == false)
				accumulator += TheStatFiles.at(i).GetTalkTime();
		}

		return accumulator;
	}

	const int NumDispos(const std::string & dispo) {

		int accumulator = 0;
		for (unsigned int i = 0; i < TheStatFiles.size(); i++) {
			if ((excludeClosers && atoi(TheStatFiles.at(i).GetAgent().c_str()) < 600) || excludeClosers == false)
				accumulator += TheStatFiles.at(i).NumDispos(dispo);
		}

		return accumulator;
	}

	const int TotalCalls() {

        	int accumulator = 0;

        	for (unsigned int i = 0; i < TheStatFiles.size(); i++) {
			if ((excludeClosers && atoi(TheStatFiles.at(i).GetAgent().c_str()) < 600) || excludeClosers == false)
	                	accumulator += TheStatFiles.at(i).size_in_range();
                }

        	return accumulator;
        }

	const int GetSales() { return NumDispos("12"); }
	const int GetSales(const std::string & agentorcampaign) { return NumDispos("12",agentorcampaign); }
	const int GetNoSales() { return NumDispos("11"); }
	const int GetNoSales(const std::string & agentorcampaign) { return NumDispos("11",agentorcampaign); }
	const int GetContacts() { return NumDispos("12") + NumDispos("11"); }
	const int GetContacts(const std::string & agentorcampaign) { return NumDispos("12",agentorcampaign) + NumDispos("11",agentorcampaign); }
	double GetConversion() {

		// Let's not do division by zero :)
		if (GetContacts()) {
			return (static_cast<double>(GetSales()) / static_cast<double>(GetContacts())) * 100.0;
		} else {
			return 0.0;
		}
	}

	double GetConversion(const std::string & agentorcampaign) {
		if (GetContacts(agentorcampaign)) {
			return (static_cast<double>(GetSales(agentorcampaign)) / static_cast<double>(GetContacts(agentorcampaign))) * 100.0;
		} else {
			return 0.0;
		}
	}

	double GetContactsPerHour() {
		if (GetContacts()) {
			return static_cast<double>(GetContacts()) / (static_cast<double>(GetTalkTime()) / 3600.0);
		} else {
			return 0.0;
		}
	}

	double GetContactsPerHour(const std::string & agentorcampaign) {
        	if (GetContacts(agentorcampaign)) {
                	return static_cast<double>(GetContacts(agentorcampaign)) / (static_cast<double>(GetTalkTime(agentorcampaign)) / 3600.0);
        	} else {
                	return 0.0;
        	}
	}

	double GetSalesPerHour() {
		if (GetSales()) {
			return static_cast<double>(GetSales()) / (static_cast<double>(GetTalkTime()) / 3600.0);
		} else {
			return 0.0;
		}
	}

	double GetSalesPerHour(const std::string & agentorcampaign) {
		if (GetSales(agentorcampaign)) {
			return static_cast<double>(GetSales(agentorcampaign)) / (static_cast<double>(GetTalkTime(agentorcampaign)) / 3600.0);
		} else {
			return 0.0;
		}
	}

	protected:

	std::vector<StatFile> TheStatFiles;
};

class CampaignComposite : public Composite {

	public:

	CampaignComposite() {}
	~CampaignComposite() {}

	void SetName(const std::string & name) { itsName = name; }

	void Parse(const AgentList & TheAgents) {

		for (int i = 0; i < TheAgents.size(); i++) {

			// marker marker marker

			//if (TheAgents.at(i).GetNumber() < 600) {

			TheStatFiles.push_back(GetStatFile(itos(TheAgents.at(i).GetNumber()),itsName));
			TheStatFiles.back().SetAgent(itos(TheAgents.at(i).GetNumber()));
			TheStatFiles.back().SetCampaign(itsName);

			//}
		}
	}

	const int GetTalkTime(const std::string & agent) {

        	int accumulator = 0;

        	for (unsigned int i = 0; i < TheStatFiles.size(); i++) {

			if (TheStatFiles.at(i).GetAgent() == agent) {
	                	accumulator += TheStatFiles.at(i).GetTalkTime();
			}
                }

        	return accumulator;
        }

	const int NumDispos(const std::string & dispo, const std::string & agent) {

        	int accumulator = 0;

        	for (unsigned int i = 0; i < TheStatFiles.size(); i++) {

			if (TheStatFiles.at(i).GetAgent() == agent) {
	                	accumulator += TheStatFiles.at(i).NumDispos(dispo);
			}
                }

        	return accumulator;
        }

	const int TotalCalls(const std::string & agent) {

		int accumulator = 0;

		for (unsigned int i = 0; i < TheStatFiles.size(); i++) {

			if (TheStatFiles.at(i).GetAgent() == agent) {
				accumulator += TheStatFiles.at(i).size_in_range();
			}
		}

		return accumulator;
	}

	private:

	std::string itsName;
};

class AgentComposite : public Composite {

	public:
	
	AgentComposite() {}
	~AgentComposite() {}

	void SetNumber(const std::string & number) { itsNumber = number; }

	void Parse(QueueList & TheQueues) {
		for (int i = 0; i < TheQueues.size(); i++) {
			TheStatFiles.push_back(GetStatFile(itsNumber,TheQueues.at(i).GetName()));
			TheStatFiles.back().SetAgent(itsNumber);
			TheStatFiles.back().SetCampaign(TheQueues.at(i).GetName());
		}
	}

	const int GetTalkTime(const std::string & campaign) {

        	int accumulator = 0;

	        for (unsigned int i = 0; i < TheStatFiles.size(); i++) {
        	        if (TheStatFiles.at(i).GetCampaign() == campaign) {
        	                accumulator += TheStatFiles.at(i).GetTalkTime();
                        }
                }

	        return accumulator;
        }

	const int NumDispos(const std::string & dispo, const std::string & campaign) {

        	int accumulator = 0;

	        for (unsigned int i = 0; i < TheStatFiles.size(); i++) {
        	        if (TheStatFiles.at(i).GetCampaign() == campaign) {
                	        accumulator += TheStatFiles.at(i).NumDispos(dispo);
                        }
                }

	        return accumulator;
        }

	const int TotalCalls(const std::string & campaign) {

        	int accumulator = 0;

	        for (unsigned int i = 0; i < TheStatFiles.size(); i++) {
                	if (TheStatFiles.at(i).GetCampaign() == campaign) {
                        	accumulator += TheStatFiles.at(i).size_in_range();
                        }
                }
        
		return accumulator;
        }

	private:

	std::string itsNumber;
};

const std::string formatDouble(const double & theDouble) {

	std::ostringstream DoubleStream;

	DoubleStream << std::setiosflags(std::ios_base::fixed) << std::setprecision(2) << theDouble;
	return DoubleStream.str();
}

double secsToHours(const long int & secs) {
	return static_cast<double>(secs) / 3600.0;
}

std::string formatHours(const long int & secs) {
	return formatDouble(secsToHours(secs));
}

#endif
