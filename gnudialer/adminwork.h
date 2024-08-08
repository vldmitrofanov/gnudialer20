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
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include "Socket.h"
#include "post.h"
#include "etcinfo.h"


bool campaignIsRunning(std::string campaign) {

	Queue TheQueue;
	u_long serverId = 1;
	TheQueue.ParseQueue(campaign,serverId);
	if (TheQueue.GetSetting("active").Get() == "true") {
		return true;
	} else {
		return false;
	}
}

bool dialerIsRunning() { return true; }

bool reloadQueues() {

	return true;

}

bool reloadAgents() {
	std::string response;
	ClientSocket AsteriskManager(getMainHost(),5038);
	AsteriskManager.setRecvTimeout(1000);
	AsteriskManager >> response;
        AsteriskManager << "Action: Login\r\nUserName: " + \
                        getManagerUsername() + \
                        "\r\nSecret: " + \
                        getManagerPassword() + \
                        "\r\nEvents: off\r\n\r\n";
	AsteriskManager >> response;
	AsteriskManager << "Action: Command\r\n";
	AsteriskManager << "Command: reload chan_agent.so\r\n\r\n";
	AsteriskManager >> response;
	AsteriskManager << "Action: Logoff\r\n\r\n";
	AsteriskManager >> response;

	return true;
}


bool sendUserEvent(std::string eventname, std::string eventbody) {
        std::string response;
	ClientSocket AsteriskManager(getMainHost(),5038);
	AsteriskManager >> response;
	AsteriskManager << "Action: Login\r\nUserName: " + \
			getManagerUsername() + \
			"\r\nSecret: " + \
			getManagerPassword() + \
			"\r\nEvents: off\r\n\r\n";
	AsteriskManager >> response;

	AsteriskManager << "Action: SendEvent\r\n";
	AsteriskManager << "Event: " + eventname + "\r\n";
	AsteriskManager << "Body: " + eventbody + "\r\n\r\n";

	AsteriskManager >> response;
	AsteriskManager << "Action: Logoff\r\n\r\n";
	AsteriskManager >> response;

        return true;
}


bool createQueue(std::string queue) {

	bool success;

	std::ofstream QueuesOut;
	QueuesOut.open("/etc/asterisk/queues.conf",std::ios::app);
	if (QueuesOut) success = true;
	QueuesOut << std::endl << std::endl << "[" << queue << "]" << std::endl;
	QueuesOut << "eventwhencalled = yes" << std::endl;
	QueuesOut << "strategy = leastrecent" << std::endl;
	QueuesOut << "timeout = 10" << std::endl;
	QueuesOut << "wrapuptime = 1" << std::endl << std::endl;
	QueuesOut << ";:setting:active:false" << std::endl;
	QueuesOut.close();

	return reloadQueues();
}

bool existsQueue(std::string queue) {

        bool inQueue = false, success = true;
        std::vector<std::string> QueueLines;
        std::ifstream QueuesIn;
        std::ofstream QueuesOut;
        QueuesIn.open("/etc/asterisk/queues.conf");
        if (!QueuesIn) success = false;
        for (std::string tempLine; std::getline(QueuesIn,tempLine,'\n');) {
                if (tempLine.length() > 1) {
                        if (tempLine[0] == '[') {
                                inQueue = false;
                        }
                }
                if (tempLine.find("[" + queue + "]",0) != std::string::npos) {
                        if (tempLine[0] == '[') {
                                inQueue = true;
                        }
                }
                QueueLines.push_back(tempLine);
        }
        QueuesIn.close();

        return inQueue;
}

bool removeQueue(std::string queue) {

	bool inQueue = false, success = true;
	std::vector<std::string> QueueLines;
	std::ifstream QueuesIn;
	std::ofstream QueuesOut;
	QueuesIn.open("/etc/asterisk/queues.conf");
	if (!QueuesIn) success = false;
	for (std::string tempLine; std::getline(QueuesIn,tempLine,'\n');) {
		if (tempLine.length() > 1) {
			if (tempLine[0] == '[') {
				inQueue = false;
			}
		}
		if (tempLine.find("[" + queue + "]",0) != std::string::npos) {
			if (tempLine[0] == '[') {
				inQueue = true;
			}
		}
		if (!inQueue) QueueLines.push_back(tempLine);
	}
	QueuesIn.close();

	QueuesOut.open("/etc/asterisk/queues.conf");
	if (QueuesOut) success = false;
	for (unsigned int i = 0; i < QueueLines.size(); i++)
		QueuesOut << QueueLines.at(i) << std::endl;
	QueuesOut.close();

	reloadQueues();

	return success;
}

bool startQueue(std::string queue) {

	Queue TheQueue;
	u_long serverId = 1;
	TheQueue.ParseQueue(queue,serverId);
	TheQueue.SetSetting("active","true");
	TheQueue.Write();
	return true;
}

bool stopQueue(std::string queue) {

//	sendToDialer("stop " + queue);

	Queue TheQueue;
	u_long serverId = 1;
	TheQueue.ParseQueue(queue,serverId);
	TheQueue.SetSetting("active","false");
	TheQueue.Write();

	return true;
}

bool addQueueMember(int agent, std::string queue) {

	std::ostringstream AgentStream;
	AgentStream << agent;

	bool inQueue = false, inPos = false, writeComplete = false;
	std::vector<std::string> QueueLines;
	std::ifstream QueuesIn;
	std::ofstream QueuesOut;
	QueuesIn.open("/etc/asterisk/queues.conf");
	if (!QueuesIn) {
		std::cout << "Error! Can't open queue file!<br><br>\n";
	}
	for (std::string tempLine; std::getline(QueuesIn,tempLine,'\n');) {
		QueueLines.push_back(tempLine);
		if (tempLine.length() > 1) {
			if (tempLine.find("[" + queue + "]",0) != std::string::npos) {
				inQueue = true;
			}
		}
		if (tempLine.find("member => Agent/") != std::string::npos) {
			if (tempLine[0] == 'm') {
				if (inQueue) {
					inPos = true;
				}
			}
		} else {
			inPos = true;
		}
		if (inQueue && inPos) {
			if (!writeComplete) {
				QueueLines.push_back("member => Agent/" + AgentStream.str());
				writeComplete = true;
			}
		}
	}
	QueuesIn.close();

std::cout << "<pre>";

	QueuesOut.open("/etc/asterisk/queues.conf");
	if (!QueuesOut) {
		std::cout << "Error! Can't open queue file!<br><br>\n";
	}

	for (unsigned int i = 0; i < QueueLines.size(); i++) {
		QueuesOut << QueueLines.at(i) << std::endl;
	}
	QueuesOut.close();

	reloadQueues();

	std::cout << "</pre>\n";

	return true;
}

bool removeQueueMember(std::string agent, std::string queue) {

	bool inQueue = false;
	std::vector<std::string> QueueLines;
	std::ifstream QueuesIn;
	std::ofstream QueuesOut;
	QueuesIn.open("/etc/asterisk/queues.conf");
	for (std::string tempLine; std::getline(QueuesIn,tempLine,'\n'); ) {
		if (tempLine.length() > 1) {
			if (tempLine[0] == '[') inQueue = false;
		}
		if (tempLine.length() > 1) {
			if (tempLine.find(queue,0) != std::string::npos) {
				if (tempLine[0] == '[') {
					inQueue = true;
				}
			}
		}
		if (!inQueue) QueueLines.push_back(tempLine);
		else
			if (std::string(tempLine + "foo").find("member => Agent/" + agent + "foo",0) == std::string::npos) {
				QueueLines.push_back(tempLine);
			}
		}
	QueuesIn.close();

std::cout << "<pre>";

	QueuesOut.open("/etc/asterisk/queues.conf");
	for (unsigned int i = 0; i < QueueLines.size(); i++)
		QueuesOut << QueueLines.at(i) << std::endl;
	QueuesOut.close();

std::cout << "</pre>";

	return reloadQueues();
}

bool createAgent(std::string agent, std::string pass) {
	bool success = true;
	int highestAgentNumber = 299;
	int highestCloserNumber = 599;
	
	AgentList TheAgents;
	TheAgents.ParseAgentList();

	if (agent.empty() || pass.empty()) {
		std::cout << "Oops! You forgot to set the name or password!<br><br>\n";
		success = false;
	}

	for (int i = 0; i < TheAgents.size(); i++) {
		if (TheAgents.at(i).GetNumber() > highestAgentNumber && TheAgents.at(i).GetNumber() <= highestCloserNumber)
			highestAgentNumber = TheAgents.at(i).GetNumber();
		if (TheAgents.at(i).GetName() == agent) {
			std::cout << "Error: Name already taken!<br><br>\n";
			success = false;
		}
	}

	if (success) {
		std::ofstream AgentsOut;
		AgentsOut.open("/etc/asterisk/agents.conf",std::ios::app);
		if (!AgentsOut) {
			std::cout << "Error! Can't open agent file!<br><br>\n";
			success = false;
		}
		AgentsOut << "agent => " << highestAgentNumber + 1 << ",";
		AgentsOut << pass << "," << agent << std::endl;
		AgentsOut.close();
	}

	return success;
}

bool createCloser(std::string agent, std::string pass) {
        bool success = true;
//        int highestAgentNumber = 299;
	int highestCloserNumber = 599;
                
        AgentList TheAgents;
        TheAgents.ParseAgentList();

        if (agent.empty() || pass.empty()) {
                std::cout << "Oops! You forgot to set the name or password!<br><br>\n";
                success = false;
        }

        for (int i = 0; i < TheAgents.size(); i++) {
                if (TheAgents.at(i).GetNumber() > highestCloserNumber)
                        highestCloserNumber = TheAgents.at(i).GetNumber();
                if (TheAgents.at(i).GetName() == agent) {
                        std::cout << "Error: Name already taken!<br><br>\n";
                        success = false;
                }
        }

        if (success) {
                std::ofstream AgentsOut;
                AgentsOut.open("/etc/asterisk/agents.conf",std::ios::app);
                if (!AgentsOut) {
                        std::cout << "Error! Can't open agent file!<br><br>\n";
                        success = false;
                }
                AgentsOut << "agent => " << highestCloserNumber + 1 << ",";
                AgentsOut << pass << "," << agent << std::endl;
                AgentsOut.close();
        }

        return success;
}

bool deleteAgent(std::string agent) {

//	bool success = false;
	AgentList TheAgents;
	TheAgents.ParseAgentList();
	std::ofstream AgentsOut;
	std::ifstream AgentsIn;
	std::vector<std::string> AgentLines;

	AgentsIn.open("/etc/asterisk/agents.conf");
	for (std::string tempLine; std::getline(AgentsIn,tempLine,'\n');) {
		AgentLines.push_back(tempLine);
	}
	AgentsIn.close();

	AgentsOut.open("/etc/asterisk/agents.conf");

	for (unsigned int i = 0; i < AgentLines.size(); i++) {
		if (AgentLines.at(i).find(";",0) != std::string::npos)
			AgentsOut << AgentLines.at(i) << std::endl;
		else if (AgentLines.at(i).find("agent => ",0) == std::string::npos) {
			AgentsOut << AgentLines.at(i) << std::endl;
		}
	}
	for (int i = 0; i < TheAgents.size(); i++) {
		if (TheAgents.at(i).GetNumber() != atoi( agent.c_str() )) {
			AgentsOut << "agent => " << TheAgents.at(i).GetNumber() << ",";
			AgentsOut << TheAgents.at(i).GetPass() << ",";
			AgentsOut << TheAgents.at(i).GetName() << std::endl;
		}
	}

	AgentsOut.close();

	return true;
}

bool editAgent(std::string number, std::string name, std::string pass) {

	AgentList TheAgents;
	TheAgents.ParseAgentList();
	std::vector<std::string> AgentLines;
	std::ofstream AgentsOut;
	std::ifstream AgentsIn;
	AgentsIn.open("/etc/asterisk/agents.conf");
	for (std::string tempLine; std::getline(AgentsIn,tempLine,'\n');) {
		if (tempLine.substr( 0 , 10 + number.length() ) != "agent => " + number + ",")
			AgentLines.push_back(tempLine);
	}
	AgentsIn.close();

	AgentsOut.open("/etc/asterisk/agents.conf");
	for (unsigned int i = 0; i < AgentLines.size(); i++)
		AgentsOut << AgentLines.at(i) << std::endl;

	AgentsOut << "agent => " << number << "," << pass << "," << name << std::endl;
	AgentsOut.close();

	return true;
}

