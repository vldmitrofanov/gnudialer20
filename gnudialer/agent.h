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

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "itos.h"
#include "Socket.h"
#include "exceptions.h"
#include "etcinfo.h"

#ifndef AGENT
#define AGENT

class AgentList;

class Agent {

public:

Agent() {}
~Agent() {}

void ParseAgent(const std::string & dataLine) {

	int pos = dataLine.find("agent => ") + 9;
	int end = dataLine.find(",",pos);

	herNumber = atoi(dataLine.substr(pos,end-pos).c_str());

	herStatus = -2;
	herCalltime = 0;

	pos = end;
	end = dataLine.find(",",pos+1);

	herPass = dataLine.substr(pos+1,end-pos-1);

	pos = dataLine.rfind(",");

	herName = dataLine.substr(pos+1,dataLine.length()-pos-1);  

	itsCampaign = "initialized";
	itsLeadId = "0";
}

const int & GetNumber()const { return herNumber; }
const std::string & GetPass()const { return herPass; }
const std::string & GetName()const { return herName; }

void dumpData() {
	std::cout << "Number: " << GetNumber() << "\n";
	std::cout << "Name: " << GetName() << "\n";
	std::cout << "Pass: " << GetPass() << "\n";
}

void SetLeadId(const std::string & leadId) { itsLeadId = leadId; }
const std::string & GetLeadId()const { return itsLeadId; }
void SetCampaign(const std::string & campaign) { itsCampaign = campaign; }
const std::string & GetCampaign()const { return itsCampaign; }
void SetConnectedChannel(const std::string & channel) { itsConnectedChannel = channel; }
const std::string & GetConnectedChannel()const { return itsConnectedChannel; }

void SetLoggedIn() {
        timeval tv;
	gettimeofday(&tv,NULL);
	herStatus = tv.tv_sec % 1000000;
}

void writeAgentLog(const AgentList & TheAgents) {

	timeval tv;
	gettimeofday(&tv,NULL);

	time_t thetimeinsecs = tv.tv_sec;

	tm * thetime = localtime(&thetimeinsecs);

	std::ofstream CallLog;

	std::string filename = "/var/log/asterisk/gnudialer/" + \
				itos(herNumber) + "-" + \
				itsCampaign;

	std::system(("touch " + filename).c_str());
	std::system(("chmod a+rw " + filename).c_str());
        
	CallLog.open(filename.c_str(),std::ios::app);

	CallLog << itsLeadId << "," << tv.tv_sec << "," << tv.tv_sec % 1000000 - herCalltime << ",";
	CallLog << thetime->tm_mon + 1 << "-" << thetime->tm_mday << "-" << thetime->tm_year + 1900;
	CallLog << ",";
	CallLog << std::endl;

	CallLog.close();
}


long int SetOnWait(const bool & doLog, const bool & lookupcloser, const AgentList & TheAgents) {

	// which would be equivalent to OnResume and Online

	timeval tv;
	gettimeofday(&tv,NULL);

	time_t thetimeinsecs = tv.tv_sec;

	tm * thetime = localtime(&thetimeinsecs);

	std::ofstream CallLog;

	if (doLog) {

		std::string filename = "/var/log/asterisk/gnudialer/" + \
					itos(herNumber) + "-" + \
					itsCampaign;

		std::system(("touch " + filename).c_str());
        
		CallLog.open(filename.c_str(),std::ios::app);

		CallLog << itsLeadId << "," << tv.tv_sec << "," << tv.tv_sec % 1000000 - herCalltime << ",";
		CallLog << thetime->tm_mon + 1 << "-" << thetime->tm_mday << "-" << thetime->tm_year + 1900;
		CallLog << ",";
		CallLog << std::endl;

		CallLog.close();

		// this is nasty, i'll give a $1 to the first person to tell me a better way
		//std::system(("chmod a+rw " + filename).c_str());

	}

	if (herStatus != -2) {
		herStatus = tv.tv_sec % 1000000;
	}

	return herStatus - herCalltime;
}

void SetOnCall() {
	herCalltime = herStatus;
	herStatus = -1;
}
void SetOffline() {
	herStatus = -2;
}
void SetOnPause() {
	herStatus = -3;
}
void SetInCloseout() {
        herStatus = -4;
}

const long int & GetStatus()const { 
//	std::cout << "Status: " << herStatus << std::endl;
	return herStatus;
}

const long int & GetCalltime()const {
	return herCalltime;
}

private:

int herNumber;
std::string herName;
std::string herPass;
std::string itsLeadId;
std::string itsCampaign;
std::string itsConnectedChannel;
long int herStatus, herCalltime;

};

bool operator<(const Agent & lhs, const Agent & rhs) {
	if (lhs.GetNumber() < rhs.GetNumber()) return true;
	else return false;
}

Agent ReturnAgent(std::string dataLine) {
	Agent TheAgent;
	TheAgent.ParseAgent(dataLine);
	return TheAgent;
}

class AgentList {

public:

AgentList() {}
~AgentList() {}

void ParseAgentList() {

//	std::cout << "Got to beginning of ParseAgentList" << std::endl;

//	ItsAgents.clear();
	std::ifstream AgentFileIn;
	AgentFileIn.open("/etc/asterisk/agents.conf");
	for (std::string tempLine; std::getline(AgentFileIn,tempLine,'\n');) {
		if (tempLine.find("agent => ") != std::string::npos) {
			if (tempLine[0] == 'a') {
				Agent TempAgent = ReturnAgent(tempLine);
				int tempAgentNum = TempAgent.GetNumber();
				if (!this->exists(tempAgentNum)) { 
//                                        ItsAgents.push_back(ReturnAgent(tempLine));
					ItsAgents.push_back(TempAgent);
				}
//			        std::cout << "ItsAgents size: " << ItsAgents.size() << std::endl;
//        			std::cout << tempLine << std::endl;
			}
		}
	}
	AgentFileIn.close();

	std::stable_sort(ItsAgents.begin(),ItsAgents.end());
//	std::cout << "Got to end of ParseAgentList" << std::endl;
}


const int size()const { return ItsAgents.size(); }
const Agent & at(const int & whichAgent)const { return ItsAgents.at(whichAgent); }

bool exists(const int & agentNum) {
//	Agent TheAgent;
	for (unsigned int i = 0; i < ItsAgents.size(); i++) {
		if (agentNum == ItsAgents.at(i).GetNumber()) {
			return true;
		}
	}
	return false;
}

Agent & where(const int & agentNum) {
//	std::cout << "The agent number we were passed was..." << agentNum << std::endl;
//  Agent & TheAgent;
	for (unsigned int i = 0; i < ItsAgents.size(); i++) {
		if (agentNum == ItsAgents.at(i).GetNumber()) {
			return ItsAgents.at(i);
			break;
		}
	}

	std::cout << "Uh oh! We didn't find an agent... I guess we'll probably have to segfault!" << std::endl;

	throw xLoopEnd("Exception: Didn't find an agent when lookup up: " + itos(agentNum));

//  return TheAgent;
}

bool existsConnected(const std::string & channel) {
        for (unsigned int i = 0; i < ItsAgents.size(); i++) {
                if (ItsAgents.at(i).GetConnectedChannel() == channel) {
                        return true;
		}
	}
	return false;
}

Agent & whereConnected(const std::string & channel) {
	for (unsigned int i = 0; i < ItsAgents.size(); i++) {
		if (ItsAgents.at(i).GetConnectedChannel() == channel) {
			return ItsAgents.at(i);
		}
	}
}

void AddTempAgent(int num) {
	std::ostringstream AgentStream;
	AgentStream << num;
	ItsAgents.push_back(ReturnAgent("agent => " + AgentStream.str() + ",1234,temp" + AgentStream.str()));
}

void Initialize(const std::string & managerUsername, const std::string & managerPassword) {

	std::string response, completeResponse;
	ClientSocket AsteriskManager(getMainHost(),5038);
	AsteriskManager.setRecvTimeout(1000);
	AsteriskManager >> response;
	if (response.empty()) {
		std::cout << "\033[1;31m *** Asterisk Manager Access Required (check your manager.conf and if asterisk is running) *** \033[0m" << std::endl;
	} else {
	
		AsteriskManager << "Action: Login\r\nUserName: " + managerUsername + "\r\nSecret: " + managerPassword + "\r\nEvents:off\r\n\r\n";
		AsteriskManager >> response;
		AsteriskManager << "Action: Command\r\nCommand: show agents\r\n\r\n";

		while (completeResponse.find("END COMMAND",0) == std::string::npos) {
			AsteriskManager >> response;
			completeResponse += response + "\n";
		}

		AsteriskManager << "Action: Logoff\r\n\r\n";
		AsteriskManager >> response;

		std::stringstream AgentStream;
		AgentStream << completeResponse;

		for (std::string tempLine; std::getline(AgentStream,tempLine,'\n'); ) {

			if (tempLine.find("talking",0) != std::string::npos) {

//				tempLine = tempLine.substr(0,tempLine.find("\t",0));
				std::cout << "GnuDialer: Setting Agent/" << atoi(tempLine.substr(0,tempLine.find("(",0)-1).c_str()) << " on call..." << std::endl;
				where(atoi(tempLine.substr(0,tempLine.find("(",0)-1).c_str())).SetOnCall();
			}

			if (tempLine.find("idle",0) != std::string::npos) {
//				tempLine = tempLine.substr(0,tempLine.find("\t",0));
				std::cout << "GnuDialer: Setting Agent/" << atoi(tempLine.substr(0,tempLine.find("(",0)-1).c_str()) << " on wait..." << std::endl;
				where(atoi(tempLine.substr(0,tempLine.find("(",0)-1).c_str())).SetLoggedIn();
			}
		}
	}
}

private:

std::vector<Agent> ItsAgents;

};

#endif
