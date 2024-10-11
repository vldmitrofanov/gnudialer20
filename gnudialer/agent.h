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
#include <regex>
#include <nlohmann/json.hpp>
#include <set>
#include "itos.h"
#include "Socket.h"
#include "exceptions.h"
#include "etcinfo.h"
#include "DBConnection.h"
#include "Campaign.h"
#include "ParsedAgent.h"
#include "ParsedConfBridge.h"
#include "HttpClient.h"

#ifndef AGENT
#define AGENT

using json = nlohmann::json;
class AgentList;

class Agent
{

public:
	Agent() {}
	~Agent() {}

	// void ParseAgent(const std::string & dataLine) {
	void ParseAgent(ParsedAgent parsedAgent)
	{

		// int pos = dataLine.find("agent => ") + 9;
		// int end = dataLine.find(",",pos);

		herNumber = parsedAgent.herNumber; //(dataLine.substr(pos,end-pos).c_str());

		herStatus = -2;
		herCalltime = 0;

		// pos = end;
		// end = dataLine.find(",",pos+1);

		herPass = parsedAgent.herPass; //.substr(pos+1,end-pos-1);

		// pos = dataLine.rfind(",");

		herName = parsedAgent.herName; //.substr(pos+1,dataLine.length()-pos-1);

		itsCampaign = "initialized";
		itsLeadId = "0";
	}

	const int &GetNumber() const { return herNumber; }
	const std::string &GetPass() const { return herPass; }
	const std::string &GetName() const { return herName; }

	void dumpData()
	{
		std::cout << "Number: " << GetNumber() << "\n";
		std::cout << "Name: " << GetName() << "\n";
		std::cout << "Pass: " << GetPass() << "\n";
	}

	void SetLeadId(const std::string &leadId) { itsLeadId = leadId; }
	const std::string &GetLeadId() const { return itsLeadId; }
	void SetCampaign(const std::string &campaign) { itsCampaign = campaign; }
	const std::string &GetCampaign() const { return itsCampaign; }
	void SetConnectedChannel(const std::string &channel) { itsConnectedChannel = channel; }
	const std::string &GetConnectedChannel() const { return itsConnectedChannel; }
	void SetChannel(const std::string &channel) { herChannel = channel; }
	const std::string &GetChannel() const { return herChannel; }

	void SetLoggedIn()
	{
		timeval tv;
		gettimeofday(&tv, NULL);
		herStatus = tv.tv_sec % 1000000;
	}

	void writeAgentLog(const AgentList &TheAgents)
	{

		timeval tv;
		gettimeofday(&tv, NULL);

		time_t thetimeinsecs = tv.tv_sec;

		tm *thetime = localtime(&thetimeinsecs);

		std::ofstream CallLog;

		std::string filename = "/var/log/asterisk/gnudialer/" +
							   itos(herNumber) + "-" +
							   itsCampaign;

		std::system(("touch " + filename).c_str());
		std::system(("chmod a+rw " + filename).c_str());

		CallLog.open(filename.c_str(), std::ios::app);

		CallLog << itsLeadId << "," << tv.tv_sec << "," << tv.tv_sec % 1000000 - herCalltime << ",";
		CallLog << thetime->tm_mon + 1 << "-" << thetime->tm_mday << "-" << thetime->tm_year + 1900;
		CallLog << ",";
		CallLog << std::endl;

		CallLog.close();
	}

	long int SetOnWait(const bool &doLog, const bool &lookupcloser, const AgentList &TheAgents)
	{

		// which would be equivalent to OnResume and Online

		timeval tv;
		gettimeofday(&tv, NULL);

		time_t thetimeinsecs = tv.tv_sec;

		tm *thetime = localtime(&thetimeinsecs);

		std::ofstream CallLog;

		if (doLog)
		{

			std::string filename = "/var/log/asterisk/gnudialer/" +
								   itos(herNumber) + "-" +
								   itsCampaign;

			std::system(("touch " + filename).c_str());

			CallLog.open(filename.c_str(), std::ios::app);

			CallLog << itsLeadId << "," << tv.tv_sec << "," << tv.tv_sec % 1000000 - herCalltime << ",";
			CallLog << thetime->tm_mon + 1 << "-" << thetime->tm_mday << "-" << thetime->tm_year + 1900;
			CallLog << ",";
			CallLog << std::endl;

			CallLog.close();

			// this is nasty, i'll give a $1 to the first person to tell me a better way
			// std::system(("chmod a+rw " + filename).c_str());
		}

		if (herStatus != -2)
		{
			herStatus = tv.tv_sec % 1000000;
		}

		return herStatus - herCalltime;
	}

	void SetOnCall()
	{
		herCalltime = herStatus;
		herStatus = -1;
	}
	// void SetOnWait()
	//{
	//	timeval tv;
	//	gettimeofday(&tv, NULL);
	//	herStatus = tv.tv_sec % 1000000;
	// }
	void SetOffline()
	{
		herStatus = -2;
	}
	void SetOnPause()
	{
		herStatus = -3;
	}
	void SetInCloseout()
	{
		herStatus = -4;
	}

	const long int &GetStatus() const
	{
		//	std::cout << "Status: " << herStatus << std::endl;
		return herStatus;
	}

	const long int &GetCalltime() const
	{
		return herCalltime;
	}

private:
	int herNumber;
	std::string herName;
	std::string herPass;
	std::string herChannel;
	std::string itsLeadId;
	std::string itsCampaign;
	std::string itsConnectedChannel;
	long int herStatus, herCalltime;
};

bool operator<(const Agent &lhs, const Agent &rhs)
{
	if (lhs.GetNumber() < rhs.GetNumber())
		return true;
	else
		return false;
}

Agent ReturnAgent(ParsedAgent dataObj)
{
	Agent TheAgent;
	TheAgent.ParseAgent(dataObj);
	return TheAgent;
}

class AgentList
{

public:
	AgentList() {}
	~AgentList() {}

	void ParseAgentList()
	{

		//	std::cout << "Got to beginning of ParseAgentList" << std::endl;
		DBConnection dbConn;
		u_long serverId = std::stoull(getServerId());
		std::vector<ParsedAgent> agents = dbConn.getAllAgents(serverId);

		if (agents.empty())
		{
			std::cout << "No agents found in the database." << std::endl;
		}
		else
		{
			std::cout << "agent.h Agents found: " << agents.size() << " ServerID: " << serverId << std::endl;
		}

		for (const auto &agent : agents)
		{
			Agent TempAgent = ReturnAgent(agent);
			std::cout << " - Agent ID: " << TempAgent.GetNumber() << std::endl;
			int tempAgentNum = TempAgent.GetNumber();
			if (!this->exists(tempAgentNum))
			{
				ItsAgents.push_back(TempAgent);
			}
		}

		std::stable_sort(ItsAgents.begin(), ItsAgents.end());
		ParseAgentChannelStatus();
		//	std::cout << "Got to end of ParseAgentList" << std::endl;
	}

	const int size() const { return ItsAgents.size(); }
	const Agent &at(const int &whichAgent) const { return ItsAgents.at(whichAgent); }

	bool exists(const int &agentNum)
	{
		//	Agent TheAgent;
		for (unsigned int i = 0; i < ItsAgents.size(); i++)
		{
			if (agentNum == ItsAgents.at(i).GetNumber())
			{
				return true;
			}
		}
		return false;
	}

	Agent &where(const int &agentNum)
	{
		//	std::cout << "The agent number we were passed was..." << agentNum << std::endl;
		//  Agent & TheAgent;
		for (unsigned int i = 0; i < ItsAgents.size(); i++)
		{
			if (agentNum == ItsAgents.at(i).GetNumber())
			{
				return ItsAgents.at(i);
				break;
			}
		}

		std::cout << "Uh oh! We didn't find an agent... I guess we'll probably have to segfault!" << std::endl;

		throw xLoopEnd("Exception: Didn't find an agent when lookup up: " + itos(agentNum));

		//  return TheAgent;
	}

	bool existsConnected(const std::string &channel)
	{
		for (unsigned int i = 0; i < ItsAgents.size(); i++)
		{
			if (ItsAgents.at(i).GetConnectedChannel() == channel)
			{
				return true;
			}
		}
		return false;
	}

	Agent &whereConnected(const std::string &channel)
	{
		for (unsigned int i = 0; i < ItsAgents.size(); i++)
		{
			if (ItsAgents.at(i).GetConnectedChannel() == channel)
			{
				return ItsAgents.at(i);
			}
		}
	}

	void AddTempAgent(int num)
	{
		std::ostringstream AgentStream;
		AgentStream << num;
		// ItsAgents.push_back(ReturnAgent("agent => " + AgentStream.str() + ",1234,temp" + AgentStream.str()));
		ItsAgents.push_back(ReturnAgent(ParsedAgent{
			std::stoi(AgentStream.str()),
			"temp" + AgentStream.str(),
			"1234"}));
	}

	void ParseAgentQueueStatusDEPRECATED()
	{
		std::string managerUsername = getManagerUsername();
		std::string managerPassword = getManagerPassword();
		std::string response, completeResponse;
		ClientSocket AsteriskManager(getMainHost(), 5038);
		AsteriskManager.setRecvTimeout(5000);
		AsteriskManager >> response;
		if (response.empty())
		{
			std::cout << "\033[1;31m *** Asterisk Manager Access Required (check your manager.conf and if asterisk is running) *** \033[0m" << std::endl;
		}
		else
		{
			AsteriskManager << "Action: Login\r\nUserName: " + managerUsername + "\r\nSecret: " + managerPassword + "\r\nEvents:off\r\n\r\n";
			AsteriskManager >> response;
			std::cout << "[DEBUG](agent.h) Login response: " << response << std::endl;
			AsteriskManager << "Action: Command\r\nCommand: queue show\r\n\r\n";

			bool readingOutput = false;
			// while (completeResponse.find("END COMMAND", 0) == std::string::npos)
			while (true)
			{
				AsteriskManager >> response;
				if (response.empty())
				{
					std::cerr << "Empty response, possibly timed out." << std::endl;
					break;
				}
				completeResponse += response + "\n";

				if (response == "\r\n" || response == "\n" || response == "\r")
				{
					if (readingOutput)
					{ // Second empty line indicates end of command output
						std::cout << "Detected end of response based on empty line." << std::endl;
						break;
					}
					readingOutput = true; // First empty line starts tracking end
				}
				else
				{
					readingOutput = false; // Reset if the line is not empty
				}
			}

			AsteriskManager << "Action: Logoff\r\n\r\n";
			AsteriskManager >> response;

			std::stringstream AgentStream;
			AgentStream << completeResponse;

			for (std::string tempLine; std::getline(AgentStream, tempLine, '\n');)
			{
				// std::cout << "[DEBUG](agent.h) AGENT PARSING:" + tempLine << std::endl;

				// where(atoi(tempLine.substr(0, tempLine.find("(", 0) - 1).c_str())).SetOnCall();

				// where(atoi(tempLine.substr(0, tempLine.find("(", 0) - 1).c_str())).SetLoggedIn();

				std::regex pattern(R"(PJSIP/(\d+))");
				std::smatch matches;
				if (std::regex_search(tempLine, matches, pattern))
				{
					if (!matches[1].str().empty())
					{
						int extension = std::stoi(matches[1].str());

						if (tempLine.find("(In use)") != std::string::npos)
						{
							if (tempLine.find("(paused") != std::string::npos)
							{
								std::cout << "[DEBUG](agent.h) GnuDialer: Setting PJSIP/" << extension << " on pause..." << std::endl;
								where(extension).SetOnPause();
							}
							else
							{
								std::cout << "[DEBUG](agent.h) GnuDialer: Setting PJSIP/" << extension << " on wait..." << std::endl;
								where(extension).SetLoggedIn();
							}
						}
						else if (tempLine.find("(Unavailable)") != std::string::npos)
						{
							std::cout << "[DEBUG](agent.h) GnuDialer: Setting PJSIP/" << extension << " offline..." << std::endl;
							where(extension).SetOffline();
						}
					}
				}
			}
		}
	}

	void ParseAgentQueueStatus()
	{
		DBConnection dbConn;
		u_long serverId = std::stoull(getServerId());
		std::vector<ParsedConfBridge> confBridges = dbConn.getAllConfBridges(serverId);

		for (const auto &bridge : confBridges)
		{
			int agentID = bridge.agent_id;
			u_long bridgeID = bridge.id; // Bridge ID now available

			if (bridge.online == 0)
			{
				std::cout << "[DEBUG](agent.h) GnuDialer: Setting PJSIP/" << agentID << " offline..." << std::endl;
				where(agentID).SetOffline();
			}
			else if (bridge.pause == 1)
			{
				std::cout << "[DEBUG](agent.h) GnuDialer: Setting PJSIP/" << agentID << " on pause..." << std::endl;
				where(agentID).SetOnPause();
			}
			else if (bridge.available == 1)
			{
				std::cout << "[DEBUG](agent.h) GnuDialer: Setting PJSIP/" << agentID << " on wait..." << std::endl;
				where(agentID).SetOnCall(); // Assuming "on wait" means on call			
			}
			else
			{
				std::cout << "[DEBUG](agent.h) GnuDialer: Setting PJSIP/" << agentID << " on call..." << std::endl;
				where(agentID).SetLoggedIn(); // Assuming "available" means logged in
			}

			// Optionally log the bridge ID for debugging or further operations
			std::cout << "[DEBUG](agent.h) Bridge ID for agent " << agentID << " is " << bridgeID << std::endl;
		}
	}

	void ParseAgentChannelStatus()
	{
		std::string ariHost = getMainHost();
		std::string ariUser = getAriUser();
		std::string ariPass = getAriPass();
		HttpClient client(ariHost, 8088, ariUser, ariPass);
		std::string ariResponse;
		ariResponse = client.get("/ari/channels");
		std::istringstream responseStream(ariResponse);
		json jsonArray;
		responseStream >> jsonArray;
		std::regex regexPattern(R"~(\~([a-zA-Z0-9]+)-(\d+)(?:-([a-zA-Z]+))?\~)~");
		std::set<int> parsedAgentNumbers;
		// int found = 0;
		for (const auto &item : jsonArray)
		{
			std::string channelName = item["name"];
			std::size_t startPos = channelName.find("PJSIP/");
			if (startPos == std::string::npos)
			{
				continue;
			}

			// Adjust to the position where the agent number starts
			startPos += 6; // Length of "PJSIP/"

			// Find the position of the next "-"
			std::size_t endPos = channelName.find("-", startPos);
			if (endPos == std::string::npos)
			{
				std::cout << "[DEBUG](agent.h) Error: '-' not found after 'PJSIP/' in the input string." << std::endl;
				continue;
			}

			// Extract the agent number substring
			int agentNumber = atoi(channelName.substr(startPos, endPos - startPos).c_str());

			std::cout << "[DEBUG](agent.h) Found: " << channelName << ", possible agent Name: " << agentNumber << std::endl;
			if (!exists(agentNumber))
			{
				continue;
			}
			parsedAgentNumbers.insert(agentNumber);
			where(agentNumber).SetChannel(channelName);

			if (item.contains("connected") && item["connected"].is_object() && item.contains("dialplan") && item["dialplan"].is_object())
			{
				int currentStatus = where(agentNumber).GetStatus();
				// Check if "name" key exists within "connected" and if it is a string and not empty
				if (item["connected"].contains("name") &&
					item["connected"]["name"].is_string() &&
					!item["connected"]["name"].get<std::string>().empty() &&
					item["dialplan"].contains("app_name") &&
					item["dialplan"]["app_name"].get<std::string>().empty())
				{
					std::string connectedName = item["connected"]["name"];
					std::cout << "[DEBUG](agent.h) Channel: " << channelName << ", Connected Name: " << connectedName << std::endl;
					if (currentStatus != -1)
					{
						where(agentNumber).SetOnCall();
						std::smatch matches;
						if (std::regex_search(connectedName, matches, regexPattern) && matches.size() > 2)
						{
							std::string queueName = matches[1].str(); // Extract the queue name
							std::string leadId = matches[2].str();	  // Extract the lead ID
							std::cout << "[DEBUG](agent.h) Channel: " << channelName << ", Queue Name: " << queueName << ", LeadId: " << leadId << std::endl;
							where(agentNumber).SetLeadId(leadId);
							if (!queueName.empty())
							{
								where(agentNumber).SetCampaign(queueName);
							}
						}
					}
				}
				else
				{

					if (currentStatus == -2 || currentStatus == -1)
					{
						where(agentNumber).SetLoggedIn();
						where(agentNumber).SetLeadId("");
					}
					std::cout << "[DEBUG](agent.h) Channel: " << channelName << " has no valid connected name." << std::endl;
				}
			}
			else
			{
				std::cout << "[DEBUG](agent.h) Channel: " << channelName << " has no connected information." << std::endl;
			}
		}

		for (const auto &agent : ItsAgents)
		{
			int agentNumber = agent.GetNumber(); // Assuming GetNumber() returns an int
			if (parsedAgentNumbers.find(agentNumber) == parsedAgentNumbers.end())
			{
				// Agent number not found in parsedAgentNumbers
				std::cout << "[DEBUG](agent.h) Agent " << agentNumber << " not found, setting offline." << std::endl;
				where(agentNumber).SetOffline(); // Mark the agent as offline
			}
		}
	}

	void Initialize()
	{
		ParseAgentQueueStatus();
		// Since "show queue" doesn't show us current call status of the agent we'll get info via ARI
		ParseAgentChannelStatus();
	}

private:
	std::vector<Agent> ItsAgents;
};

#endif
