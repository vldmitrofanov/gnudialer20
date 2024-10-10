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

#include <iomanip>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>
#include <sys/time.h>
#include <sys/stat.h>
#include <mysql.h>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include "queue.h"
#include "Socket.h"
#include "tzfilter.h"
#include "tzpopulate.h"
#include "statepopulate.h"
#include "exceptions.h"
#include "asterisk.h"
#include "call.h"
#include "evaluate.h"
#include "etcinfo.h"
#include "itos.h"
#include "log.h"
#include "dispo.h"
#include "settings.h"
#include "adminwork.h"
#include "abnhelper.h"
#include "color.h"
#include "HttpClient.h"
#include "ConfigSingleton.h"
#include "ParsedConfBridge.h"

#define CRASH                                                          \
	do                                                                 \
	{                                                                  \
		fprintf(stderr, "!! Forcing immediate crash a-la abort !!\n"); \
		*((int *)0) = 0;                                               \
	} while (0)

#ifdef DEBUG
bool gDebug = true;
const bool debugPos = true;
#else
bool gDebug = false;
const bool debugPos = false;
#endif

#define HERE(x)                                  \
	if (debugPos)                                \
	{                                            \
		std::cout << "Here:" << #x << std::endl; \
	}

const bool debugCampaignSettings = true; // dont forget to set to false
using json = nlohmann::json;

const int &selectLessorOf(const int &lhs, const int &rhs)
{
	if (lhs < rhs)
	{
		return lhs;
	}
	else
	{
		return rhs;
	}
}

long int stoi(const std::string &theString)
{
	return atoi(theString.c_str());
}

bool safeMode;
void sig_handler(int sig)
{

	if (sig == SIGSEGV)
	{
		if (safeMode == true)
		{
			if (doColorize)
			{
				std::cout << fg_light_red << "FATAL ERROR! Segmentation Fault!" << normal << std::endl;
			}
			else
			{
				std::cout << "FATAL ERROR! Segmentation Fault!" << std::endl;
			}
			std::cout << "You are running in safe mode, so GnuDialer will attempt to restart itself!" << std::endl
					  << std::endl;
			std::system("sleep 1 && gnudialer &");
		}
		else
		{
			CRASH;
			if (doColorize)
			{
				std::cout << fg_light_red << "FATAL ERROR! Segmentation Fault!" << normal << std::endl;
			}
			else
			{
				std::cout << "FATAL ERROR! Segmentation Fault!" << std::endl;
			}
			std::cout << "Please report this to the GnuDialer project." << std::endl;
			std::cout << "Please also be advised that you can start gnudialer in \"safe\" mode which will" << std::endl;
			std::cout << "automatically restart GnuDialer if you receive a fatal error." << std::endl;
			std::cout << "Type: \"gnudialer --help\" for more information." << std::endl
					  << std::endl;
		}
		exit(0);
	}

	if (sig == SIGCHLD)
	{
		int stat;
		while (waitpid(-1, &stat, WNOHANG) > 0)
			;
	}
	return;
}

void createDispositionRecord(const std::string &agent_id, const std::string &campaign_code, const std::string &lead_id)
{
	// Initialize CURL
	CURL *curl;
	CURLcode res;

	// Prepare the API URL
	std::string url = getApiUrl() + "/api/dispositions";
	std::cout << "DEBUG:: create dispo record URL " << url << std::endl;

	// Prepare the Bearer token
	std::string bearer_token = "Bearer " + getApiUserSecret();

	// Initialize the CURL object
	curl = curl_easy_init();
	if (curl)
	{
		// Set the URL for the request
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Set the HTTP header with the Bearer token
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, ("Authorization: " + bearer_token).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// Prepare the POST data (JSON)
		std::string json_data = "{\"campaign_code\": \"" + campaign_code + "\","
																		   "\"agent_id\": \"" +
								agent_id + "\","
										   "\"lead_id\": \"" +
								lead_id + "\"}";

		// Set the POST fields
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

		// Send the request and get the response
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK)
		{
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
		}
		else
		{
			std::cout << "Request sent successfully!" << std::endl;
		}

		// Clean up
		curl_slist_free_all(headers); // Free headers
		curl_easy_cleanup(curl);	  // Clean up CURL instance
	}
	else
	{
		std::cerr << "Failed to initialize CURL." << std::endl;
	}
}

std::string trim(const std::string &str)
{
	size_t first = str.find_first_not_of(" \n\r\t");
	size_t last = str.find_last_not_of(" \n\r\t");
	if (first == std::string::npos || last == std::string::npos)
	{
		return ""; // Return empty string if no valid characters
	}
	return str.substr(first, (last - first + 1));
}

void doAriRedirect(const std::string &channel,
				   const std::string &agent,
				   const std::string &campaign,
				   const std::string &leadid,
				   const bool &doChangeCallerId,
				   AgentList &TheAgents)
{
	std::string ariHost = getMainHost();
	std::string ariUser = getAriUser();
	std::string ariPass = getAriPass();
	std::string managerUser = getManagerUser();
	std::string managerPass = getManagerPass();

	HttpClient client(ariHost, 8088, ariUser, ariPass); // You need to implement or use an existing HTTP client library
	std::string response;

	u_long serverId = std::stoull(getServerId());

	DBConnection dbConn;
	u_long agentId = std::stoul(agent.c_str());
	u_long bridgeDbId = dbConn.getConfBridgeIdForAgent(agentId, serverId);

	if (bridgeDbId == 0)
	{
		std::cerr << "ERROR: No bridge found for agent: " << agent << std::endl;
		return;
	}

	std::string bridgeName = std::to_string(bridgeDbId);

	if (atoi(agent.c_str()) || (agent == "699" && doChangeCallerId))
	{

		if (doColorize)
		{
			std::cout << "[DEBUG]" << campaign << fg_magenta << ": Transferring - " << channel << " to Agent: " << agent << normal << std::endl;
		}
		else
		{
			std::cout << "[DEBUG]" << campaign << ": Transferring - " << channel << " to Agent: " << agent << std::endl;
		}

		writeGnudialerLog(campaign + ": Transferring - " + channel + " to Agent: " + agent + "");
		response = client.get("/ari/bridges");
		std::istringstream responseStream(response);
		json bridgesJsonArray;
		responseStream >> bridgesJsonArray;

		std::string bridgeId;
		std::string agentChannel;
		bool bridgeFound = false;

		for (const auto &bridge : bridgesJsonArray)
		{
			std::string currentBridgeName = bridge["name"];
			if (currentBridgeName == bridgeName)
			{
				bridgeId = bridge["id"];
				std::cout << "[DEBUG] Found matching bridge: " << bridgeName << " (ID: " << bridgeId << ")" << std::endl;

				for (const auto &channelId : bridge["channels"])
				{
					// Step 5: Get channel details to map channel ID to channel name (PJSIP/...)
					std::string channelResponse = client.get("/ari/channels/" + std::string(channelId));
					std::istringstream channelStream(channelResponse);
					json channelDetails;
					channelStream >> channelDetails;

					std::string fullChannelName = channelDetails["name"]; // Full name like "PJSIP/10001-xxxxxxxx"

					// Step 6: Check if the channel belongs to the agent
					if (fullChannelName.find("PJSIP/" + agent + "-") != std::string::npos)
					{
						agentChannel = fullChannelName; // Store the agent's channel name
						bridgeFound = true;				// Mark that we found the right bridge
						break;
					}
				}
				break;
			}
		}

		if (!bridgeFound)
		{
			std::cerr << "[ERROR] No matching bridge found for name: " << bridgeName << std::endl;
			return;
		}

		if (!agentChannel.empty())
		{
			std::string bridgeName = std::to_string(bridgeDbId);
			if (doColorize)
			{
				std::cout << "[DEBUG]" << fg_light_yellow << campaign << ": Bridging - " << channel << " to Agent's channel: " << agentChannel << " in a confBridge " << bridgeName << " Bridge ID: " << bridgeId << normal << std::endl;
			}
			// I use AMI to join agent's conf bridge, because
			// of the eror that channel is not part os stasis app
			ClientSocket AsteriskManager(getMainHost(), 5038);
			AsteriskManager >> response;
			AsteriskManager << "Action: Login\r\nUserName: " + managerUser + "\r\nSecret: " + managerPass + "\r\nEvents:off\r\n\r\n";
			AsteriskManager >> response;

			AsteriskManager << "Action: Redirect\r\n";
			AsteriskManager << "Channel: " + channel + "\r\n";						  // Channel to redirect
			AsteriskManager << "Context: conf_bridge_context\r\n";					  // Dialplan context
			AsteriskManager << "Exten: s\r\n";										  // Extension to handle the conference join
			AsteriskManager << "Priority: 1\r\n";									  // Priority in dialplan
			AsteriskManager << "Variable: CONF_BRIDGE_ID=" + bridgeName + "\r\n"; // Pass the ConfBridge ID as a variable
			AsteriskManager << "\r\n";
			AsteriskManager >> response;
			std::cout << "ConfBridge Response: " << response << std::endl;

			AsteriskManager << "Action: Logoff\r\n\r\n";
			AsteriskManager >> response;

			createDispositionRecord(agent, campaign, leadid);
			usleep(10000000);
		}
		else
		{
			if (doColorize)
			{
				std::cout << campaign << fg_red << ": ERROR - " << channel << " to Agent's channel NOT FOUND! Agent: " << agent << normal << std::endl;
			}
			else
			{
				std::cout << campaign << ": ERROR - " << channel << " to Agent's channel NOT FOUND: " << agent << std::endl;
			}
		}
	}
}

void doRedirect(const std::string &channel,
				const std::string &agent,
				const std::string &campaign,
				const std::string &leadid,
				const bool &doChangeCallerId)
{
	std::string managerUser = getManagerUser();
	std::string managerPass = getManagerPass();
	std::string response;
	if (atoi(agent.c_str()) || agent == "699" && doChangeCallerId)
	{
		if (doColorize)
		{
			std::cout << campaign << fg_magenta << ": Transferring - " << channel << " to Agent: " << agent << normal << std::endl;
		}
		else
		{
			std::cout << campaign << ": Transferring - " << channel << " to Agent: " << agent << std::endl;
		}
		writeGnudialerLog(campaign + ": Transferring - " + channel + " to Agent: " + agent + "");
		ClientSocket AsteriskRedir(getMainHost(), 5038);
		AsteriskRedir >> response;
		AsteriskRedir << "Action: Login\r\nUserName: " + managerUser + "\r\nSecret: " + managerPass + "\r\nEvents:off\r\n\r\n";
		AsteriskRedir >> response;
		// AsteriskRedir << "Action: Redirect\r\n";
		// AsteriskRedir << "Channel: " + channel + "\r\n";
		// AsteriskRedir << "Exten: " + agent + "\r\n";

		std::string line;
		std::string currentChannel;
		std::string fullResponse;
		bool inChannelBlock = false;

		AsteriskRedir << "Action: CoreShowChannels\r\n\r\n";
		AsteriskRedir >> response;

		while (true)
		{
			AsteriskRedir >> response;
			fullResponse += response;

			// Check if we've reached the end of the response
			if (response.find("EventList: Complete") != std::string::npos)
			{
				break;
			}
		}
		std::istringstream responseStream(fullResponse);
		while (std::getline(responseStream, line))
		{

			// Trim the line
			line.erase(line.find_last_not_of(" \n\r\t") + 1);
			std::cout << "Line: " << line << std::endl; // Debugging output
			if (line.empty())
			{
				// End of an event block, reset variables
				inChannelBlock = false;
				currentChannel.clear();
				continue;
			}

			if (line.find("Event: CoreShowChannel") != std::string::npos)
			{
				inChannelBlock = true;
			}

			if (inChannelBlock)
			{
				if (line.find("Channel: ") != std::string::npos)
				{
					currentChannel = line.substr(line.find(": ") + 2);
				}

				std::cout << "Found channel: " << currentChannel << " Agent: " << agent << std::endl;

				// Check if this is the agent we are looking for
				if (!currentChannel.empty() && currentChannel.find("PJSIP/" + agent) != std::string::npos)
				{
					break;
				}
			}
			if (line.find("EventList: Complete") != std::string::npos)
			{
				break;
			}
		}

		if (!currentChannel.empty())
		{
			if (doColorize)
			{
				std::cout << campaign << fg_magenta << ": Bridging - " << channel << " to Agent's channel: " << currentChannel << normal << std::endl;
			}
			else
			{
				std::cout << campaign << ": Bridging - " << channel << " to Agent's channel: " << currentChannel << std::endl;
			}
			AsteriskRedir << "Action: Bridge\r\n";
			AsteriskRedir << "Channel1: " + channel + "\r\n";
			AsteriskRedir << "Channel2: " + currentChannel + "\r\n";
			AsteriskRedir << "Tone: yes\r\n\r\n";
			AsteriskRedir >> response;
			std::cout << "Bridge Response: " << response << std::endl;

			AsteriskRedir << "Action: UserEvent\r\n";
			AsteriskRedir << "UserEvent: SetOnCall\r\n";
			AsteriskRedir << "Header: Agent: " + agent + "\r\n\r\n";
			AsteriskRedir >> response;
		}
		else
		{
			if (doColorize)
			{
				std::cout << campaign << fg_red << ": ERROR - " << channel << " to Agent's channel NOT FOUND: " << agent << normal << std::endl;
			}
			else
			{
				std::cout << campaign << ": ERROR - " << channel << " to Agent's channel NOT FOUND: " << agent << std::endl;
			}
		}

		/**
				if (!doChangeCallerId)
				{
					AsteriskRedir << "Context: agent\r\n";
				}
				else
				{
					AsteriskRedir << "Context: closer\r\n";
				}
		*/
		// AsteriskRedir << "Priority: 1\r\n\r\n";
		// AsteriskRedir >> response;
		// std::cout << "Redirect Response: " << response << std::endl;

		AsteriskRedir << "Action: Logoff\r\n\r\n";
		AsteriskRedir >> response;
		usleep(10000000);
	}
}

void doPrintSale(const std::string &theText,
				 const std::string &theCampaign,
				 const std::string &theLeadid)
{
	usleep(3000000);

	writeCampaignSale(theText, theCampaign, theLeadid);
}

void doSetVarTransfer(const std::string &channel,
					  const std::string &agent,
					  const std::string &managerUser,
					  const std::string &managerPass)
{
	std::string response;
	std::cout << "doSetVarTransfer - " << channel << std::endl;
	// writeGnudialerLog(campaign + ": doSetVarTransfer - " + channel + " from Agent: " + agent + "");
	ClientSocket AsteriskRedir(getMainHost(), 5038);
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Login\r\nUserName: " + managerUser + "\r\nSecret: " + managerPass + "\r\nEvents:off\r\n\r\n";
	AsteriskRedir >> response;
	// action: SetVar
	// channel: Zap/49-1
	// variable: ISTRANSFER=TRANSFER
	AsteriskRedir << "Action: SetVar\r\n";
	AsteriskRedir << "Channel: " + channel + "\r\n";
	AsteriskRedir << "Variable: ISTRANSFER=TRANSFER\r\n\r\n";
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Logoff\r\n\r\n";
	AsteriskRedir >> response;
	if (response == "Success")
	{
		std::cout << "doSetVarTransfer - SUCCESSFUL " << channel << std::endl;
	}
	AsteriskRedir >> response;
}

void doHangupCall(const std::string &channel,
				  const std::string &agent,
				  const std::string &managerUser,
				  const std::string &managerPass)
{
	std::string response;
	std::cout << "doHangupCall - " << channel << std::endl;
	writeGnudialerLog("doHangupCall - " + channel + " from Agent: " + agent + "");
	ClientSocket AsteriskRedir(getMainHost(), 5038);
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Login\r\nUserName: " + managerUser + "\r\nSecret: " + managerPass + "\r\nEvents:off\r\n\r\n";
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Hangup\r\n";
	AsteriskRedir << "Channel: " + channel + "\r\n\r\n";
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Logoff\r\n\r\n";
	AsteriskRedir >> response;
}

void doMonitorStart(const std::string &channel,
					const std::string &agent,
					const std::string &campaign,
					const std::string &leadid,
					const std::string &managerUser,
					const std::string &managerPass)
{
	std::string response;
	std::cout << "doMonitorStart - " << channel << std::endl;
	writeGnudialerLog("doMonitorStart - " + channel + " from Agent: " + agent + "");
	ClientSocket AsteriskRedir(getMainHost(), 5038);
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Login\r\nUserName: " + managerUser + "\r\nSecret: " + managerPass + "\r\nEvents:off\r\n\r\n";
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Monitor\r\n";
	AsteriskRedir << "Channel: " + channel + "\r\n";
	AsteriskRedir << "File: " + agent + "-" + campaign + "-" + leadid + "\r\n";
	AsteriskRedir << "Mix: 1\r\n\r\n";
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Logoff\r\n\r\n";
	AsteriskRedir >> response;
}

void doMonitorStop(const std::string &channel,
				   const std::string &agent,
				   const std::string &campaign,
				   const std::string &leadid,
				   const std::string &managerUser,
				   const std::string &managerPass)
{
	std::string response;
	std::cout << "doMonitorStop - " << channel << std::endl;
	writeGnudialerLog("doMonitorStop - " + channel + " from Agent: " + agent + "");
	ClientSocket AsteriskRedir(getMainHost(), 5038);
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Login\r\nUserName: " + managerUser + "\r\nSecret: " + managerPass + "\r\nEvents:off\r\n\r\n";
	AsteriskRedir >> response;
	AsteriskRedir << "Action: StopMonitor\r\n";
	AsteriskRedir << "Channel: " + channel + "\r\n\r\n";
	AsteriskRedir >> response;
	AsteriskRedir << "Action: Logoff\r\n\r\n";
	AsteriskRedir >> response;
}

int main(int argc, char **argv)
{
	usleep(100000);
	safeMode = false;
	bool daemonMode = true;
	MYSQL_RES *queryResult;
	MYSQL_ROW queryRow;

	signal(SIGCHLD, sig_handler);
	signal(SIGSEGV, sig_handler);

	// set default console color to white on black
	if (doColorize)
	{
		std::cout << fg_light_white << std::endl;
	}

	for (int i = 1; i < argc; ++i)
	{
		const std::string arg(argv[i]);

		if (arg == "stop" ||
			arg == "-stop" ||
			arg == "--stop")
		{
			writeGnudialerLog("GnuDialer: Stopped");
			if (doColorize)
			{
				std::cout << fg_light_green << "GnuDialer: Stopped" << normal << std::endl;
			}
			else
			{
				std::cout << "GnuDialer: Stopped" << std::endl;
			}
			std::system(("killall " + std::string(argv[0])).c_str());
			exit(0);
		}
		if (arg == "--tzpopulate" ||
			arg == "tzpopulate" ||
			arg == "-tzpopulate")
		{
			if (i + 1 < argc)
			{
				tzpopulate(argv[i + 1]);
				statepopulate(argv[i + 1]);
				return 0;
			}
			else
			{
				std::cerr << "GnuDialer: tzpopulate - Error! Missing campaign name!" << std::endl;
				return 1;
			}
		}
		if (arg == "-h" ||
			arg == "--help" ||
			arg == "-help")
		{
			std::cout << std::endl
					  << std::endl;
			std::cout << "Usage: gnudialer <options>" << std::endl
					  << std::endl;
			std::cout << "\t--safe                         Starts gnudialer in \"safe\" mode which makes in auto-restart on fatal error." << std::endl;
			std::cout << "\t--tzpopulate <campaign name>   Will create and populate the fields necessary for a campaign." << std::endl;
			std::cout << "\t--help                         This help screen." << std::endl;
			std::cout << "\t--stop                         Unconditionally stop the dialer." << std::endl;
			std::cout << "\t--no-daemon                    Do not run as a daemon." << std::endl;
			std::cout << std::endl
					  << std::endl;
			return 0;
		}
		if (arg == "-s" ||
			arg == "--safe" ||
			arg == "-safe")
		{
			std::cout << "GnuDialer: SAFE MODE enabled!" << std::endl;
			safeMode = true;
		}
		if (arg == "-nd" ||
			arg == "--no-daemon" ||
			arg == "-no-daemon")
		{
			daemonMode = false;
		}
	}

	if (daemonMode)
	{
		int daemonizer = 0;
		daemonizer = fork();
		if (daemonizer < 0)
		{
			std::cout << "GnuDialer: Error setting up daemon process... Aborting." << std::endl;
			exit(1);
		}
		if (daemonizer > 0)
		{
			exit(0);
		}
	}

	umask(017);
	chdir("/tmp");
	int lfp = open("gnudialer.lock", O_RDWR | O_CREAT, 0660);
	if (lfp < 0)
	{
		std::cout << "GnuDialer: Error opening lock file!" << std::endl;
		exit(1);
	}
	if (lockf(lfp, F_TLOCK, 0) < 0)
	{
		std::cout << "GnuDialer: process already running!" << std::endl;
		exit(0);
	}
	char str[80];
	sprintf(str, "%d\n", getpid());
	write(lfp, str, strlen(str));

	bool isAHoliday;
	try
	{
		isAHoliday = isHoliday();
	}
	catch (const xFileOpenError &e)
	{
		std::cerr << "GnuDialer: Exception! Unable to open " << e.GetFilename() << "!" << std::endl;
		return 1;
	}

	// this creates a CLOSER campaign is if doesn't exist
	// did this since people are forgetting to create it
	// if (!existsQueue("CLOSER")) { createQueue("CLOSER"); }

	HERE(ABOUT TO INITIALIZE GLOBAL SETTINGS)
	bool gLog;
	addGlobalSettings("general");
	Queue TheQueueGlobals;
	u_long serverId = 1;
	TheQueueGlobals.ParseQueue("general", serverId);

	try
	{
		gDebug = TheQueueGlobals.GetSetting("debug").GetBool();
		gLog = TheQueueGlobals.GetSetting("log").GetBool();
	}
	catch (xLoopEnd e)
	{
		std::cout << "Caught xLoopEnd while getting TheQueueGlobals..." << std::endl;
		std::cout << e.what();
		std::cout << std::endl
				  << std::endl;
	}

	std::string mainHost = getMainHost();

	HERE(MYSQLBEGIN)
	MYSQL *mysql = NULL;
	MYSQL_RES *result;
	MYSQL_ROW row;
	HERE(MYSQLEND)
	mysql = mysql_init(NULL);
	if (mysql == NULL)
	{
		std::cerr << "GnuDialer: MySql init failed!" << std::endl;
		return 1;
	}
	if (!mysql_real_connect(mysql, getMySqlHost().c_str(),
							getMySqlUser().c_str(),
							getMySqlPass().c_str(),
							getDbName().c_str(),
							3306, NULL, 0))
	{
		std::cerr << "GnuDialer: MySql connection failed!" << std::endl;
		return 1;
	}
	try
	{
		// Not nested for convenience.
		if (gLog)
		{
			writeGnudialerLog("GnuDialer: Started");
		}
		if (gDebug)
		{
			if (doColorize)
			{
				std::cout << fg_light_green << "GnuDialer: Started" << normal << std::endl;
			}
			else
			{
				std::cout << "GnuDialer: Started" << std::endl;
			}
			std::cout << "GnuDialer: loading gdhosts.conf" << std::endl;
		}
		CallCache *TheCallCache;
		try
		{
			TheCallCache = new CallCache();
		}
		catch (xTooFewFields)
		{
			std::cerr << "Exception: Too few fields in gdhosts.conf!" << std::endl;
			return 1;
		}
		catch (xTooManyFields)
		{
			std::cerr << "Exception: Too many fields in gdhosts.conf!" << std::endl;
			return 1;
		}
		catch (xInvalidWeightValue)
		{
			std::cerr << "Exception: Invalid weight value in gdhosts.conf!" << std::endl;
			return 1;
		}
		catch (const xFileOpenError &e)
		{
			std::cerr << "Exception: Error opening " << e.GetFilename() << "!" << std::endl;
			return 1;
		}
		catch (xNoHostsDefined)
		{
			std::cerr << "Exception: No hosts defined in gdhosts.conf!" << std::endl;
			return 1;
		}
		if (gDebug)
		{
			std::cout << "GnuDialer: loading gdhosts.conf (done)" << std::endl;
		}

		std::string response, block, queue, mode, calltoday, usednc, query, tzearliest, tzlatest, callerid, channel;
		std::string tempagent, usecloser, closercam;
		std::string dspmode, trunk, dialprefix, transfer, filter, usecallback, usetzfilter;
		bool debug;
		//	bool log;
		int skip;
		std::string f_areacode, f_areacode_prefix, f_zipcode, orderby;
		std::string managerUser = getManagerUsername();
		std::string managerPass = getManagerPassword();
		std::stringstream BlockStream;
		unsigned int maxlines = 0, timeout = 0, linesdialing = 0, availagents = 0, linestodial = 0, counter = 0;
		unsigned int pos = 0, end = 0, pos2 = 0, end2 = 0;
		unsigned long int calls = 0, abandons = 0;
		int pid = 0;
		double maxratio = 0.0, maxabandons = 0.0;

		ClientSocket AsteriskManager(getMainHost(), 5038);
		AsteriskManager.setRecvTimeout(1000);
		AsteriskManager >> response;
		AsteriskManager << "Action: login\r\nUsername: " + managerUser + "\r\nSecret: " + managerPass + "\r\n\r\n";
		AsteriskManager >> response;

		HERE(ABOUT TO INITIALIZE QUEUES AND AGENTS)
		QueueList TheQueues;
		AgentList TheAgents;
		TheQueues.ParseQueues();
		TheAgents.ParseAgentList();
		if (gDebug)
		{
			std::cout << "GnuDialer: Querying Asterisk Agents" << std::endl;
		}

		TheAgents.Initialize();

		if (gDebug)
		{
			std::cout << "GnuDialer: Querying Asterisk Agents (done)" << std::endl;
		}
		HERE(SETTINGS CHECK)
		std::string tempCheckCampaign;
		for (int i = 0; i < TheQueues.size(); i++)
		{
			tempCheckCampaign = TheQueues.at(i).GetName();
			if (gDebug)
			{
				std::cout << tempCheckCampaign << ": Settings Pre-Check " << std::endl;
			}
			addBasicSettings(tempCheckCampaign);

			if (gDebug)
			{
				std::cout << tempCheckCampaign << ": Resetting Filters " << std::endl;
			}
			resetFilterSettings(tempCheckCampaign);
		}
		// TheQueues.ParseQueues(); // Temporary disabled

		HERE(INITIALIZED QUEUES AND AGENTS)
		timeval tv;
		unsigned long int timeSinceLastQueueUpdate = 0, timeSinceLastCallbackCheck = 0, currentTime = 0;

		for (unsigned long int t = 0; true; t++)
		{
			if (gDebug)
			{
				std::cout << "INITIALIZED QUEUES AND AGENTS FOR LOOP" << std::endl;
			}
			gettimeofday(&tv, NULL);
			currentTime = tv.tv_sec % 1000000;
			if ((t != 0 && t % 10 == 0 && currentTime - timeSinceLastQueueUpdate > 5) || (currentTime - timeSinceLastQueueUpdate > 20 && t != 0))
			{
				if (gDebug)
				{
					if (doColorize)
					{
						std::cout << "GnuDialer: " << fg_light_yellow << "Updating Campaign Settings " << fg_light_cyan << "(" << currentTime << ")" << normal << std::endl;
					}
					else
					{
						std::cout << "GnuDialer: Updating Campaign Settings " << "(" << currentTime << ")" << std::endl;
					}
				}
				TheAgents.ParseAgentList();
				TheQueues.ParseQueues();
				//			Queue TheQueueGlobals;
				u_long serverId = 1;
				TheQueueGlobals.ParseQueue("general", serverId);
				/*
				try
				{
					gDebug = TheQueueGlobals.GetSetting("debug").GetBool();
					gLog = TheQueueGlobals.GetSetting("log").GetBool();
				}
				catch (xLoopEnd e)
				{
					std::cout << "Caught exception while trying to get debug & log settings!" << std::endl;
					std::cout << e.what();
					std::cout << std::endl
							  << std::endl;
				}
				*/

				timeSinceLastQueueUpdate = currentTime;
			}
			response = "";
			AsteriskManager >> response;
			if (response.empty())
			{
				response = "Event: NoEvent\r\n\r\n";
			}
			BlockStream.clear();
			BlockStream.str(response);

			// TRIGGER LOOP
			for (std::string tempLine; std::getline(BlockStream, tempLine, '\n');)
			{
				tempLine = tempLine.substr(0, tempLine.length() - 1);

				// DEBUGGING AMI OUTPUT
				std::string command = "echo \"" + tempLine + "\" >> /tmp/gnudialer-ami.log";
				int result = system(command.c_str());
				// END DEBUGGING AMI OUTPUT

				// strip '\r'
				if (tempLine.empty())
				{
					pos = 0;
					end = 0;
					pos2 = 0;
					end2 = 0;
					// Begin block analysis
					// const std::string param(const std::string & block, const std::string & type) {
					//         int pos = 0, len = 0;
					//         if (block.find(type + ": ",0) == std::string::npos) {
					//                 return static_cast<const std::string>("");
					//         } else {
					//                 pos = block.find(type + ": ",0) + 2;
					//                 len = type.length();
					//                 return block.substr(pos + len,block.find("\n",pos) - (pos + len));
					//                 }
					//         }
					//***********************************************************************************

					// THIS CAUSES GNUDIALER TO BE KILLED WHEN ASTERISK WAS SHUTDOWN
					// THIS DOES NOT HANDLE OTHER ASTERISK EXITS, LIKE CORES
					if (block.find("Event: Shutdown", 0) != std::string::npos)
					{
						if (gDebug)
						{
							std::cout << "GnuDialer: Asterisk Shutdown - GnuDialer Killed" << std::endl;
						}
						if (gLog)
						{
							writeGnudialerLog("GnuDialer: Asterisk Shutdown - GnuDialer Killed");
						}
						std::system("killall gnudialer");
					}

					if (block.find("Event: QueueMemberPause", 0) != std::string::npos)
					{
						std::istringstream blockStream(block); // Convert the string to an input stream
						std::string line;
						std::string interface;
						std::string agentIDQP;
						std::string pausedString;
						bool isPaused = false;
						if (doColorize)
						{
							std::cout << fg_light_blue << "QueueMemberPause EVENT" << normal << std::endl;
						}
						for (std::string line; std::getline(blockStream, line);)
						{
							// Find and extract the "Interface" line
							size_t interfacePos = line.find("Interface: ", 0);
							size_t pausedPos = line.find("Paused: 1", 0);
							if (interfacePos != std::string::npos)
							{
								interface = line.substr(interfacePos + 11); // 11 is the length of "Interface: "
								// std::cout << "Interface: " << interface << std::endl;

								// Extract agent ID from the Interface
								size_t pos = interface.find('/');
								if (pos != std::string::npos && pos + 1 < interface.size())
								{
									agentIDQP = interface.substr(pos + 1);
									if (doColorize)
									{
										std::cout << fg_light_blue << "Agent ID:(" << agentIDQP << ")" << normal << std::endl;
									}
								}
							}
							else if (pausedPos != std::string::npos)
							{
								isPaused = true;
							}
						}
						if (!agentIDQP.empty())
						{
							if (isPaused)
							{
								TheAgents.where(atoi(agentIDQP.c_str())).SetOnPause();
								std::cout << fg_light_blue << "[DEBUG] Agent ID: " << agentIDQP << " is PAUSED" << normal << std::endl;
							}
							else
							{
								int currStatus = TheAgents.where(atoi(agentIDQP.c_str())).GetStatus();
								if (currStatus == -3)
								{
									TheAgents.where(atoi(agentIDQP.c_str())).SetLoggedIn();
									std::cout << "[DEBUG] Agent ID: " << agentIDQP << " on WAIT" << std::endl;
								}
								else
								{
									std::cout << "[DEBUG] Agent ID: " << agentIDQP << " status was not changed" << std::endl;
								}
							}
						}
					}

					if (block.find("Event: OriginateResponse", 0) != std::string::npos && block.find("Context: gdincoming", 0) != std::string::npos)
					{
						//***********************************************************************************
						if (block.find("Response: Failure", 0) != std::string::npos)
						{
							// if (block.find("Event: OriginateFailure",0) != std::string::npos) {
							int iTheReason = 0;
							std::string theReason, theCallerIDName, theCampaign, theLeadid;
							std::string theDispo = "0";
							std::string theReasonDesc = "Unknown";

							if (block.find("Reason: ", 0) != std::string::npos)
							{
								pos = block.find("Reason: ", 0) + 8;
								end = block.find("\n", pos);
								theReason = block.substr(pos, end - pos);
								iTheReason = atoi(theReason.c_str());
							}
							if (block.find("CallerIDName: ", 0) != std::string::npos && block.find("~", 0) != std::string::npos && !theReason.empty())
							{
								pos = block.find("CallerIDName: ", 0) + 15;
								end = block.find("\n", pos);
								theCallerIDName = block.substr(pos, end - pos);

								if (block.find("~", 0) != std::string::npos)
								{
									pos = theCallerIDName.find("~", end) + 1;
									end = theCallerIDName.find("-", pos + 1);
									pos2 = end + 1;
									end2 = theCallerIDName.find("-", pos2);
									theCampaign = theCallerIDName.substr(pos, end - pos);
									theLeadid = theCallerIDName.substr(pos2, end2 - pos2);
								}
								if (!theCampaign.empty() && !theLeadid.empty() && TheQueues.exists(theCampaign))
								{
									// unknown failure, disconnect
									if (theReason == "0")
									{
										theDispo = "-7";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
										TheQueues.rWhere(theCampaign).IncrementDisconnects();
									}
									// AST_CONTROL_RINGING
									// timed out while ringing, no answer
									if (theReason == "3")
									{
										theDispo = "-2";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
										TheQueues.rWhere(theCampaign).IncrementNoanswers();
									}
									// AST_CONTROL_BUSY
									// busy
									if (theReason == "5")
									{
										theDispo = "-4";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
										TheQueues.rWhere(theCampaign).IncrementBusies();
									}
									// AST_CONTROL_HANGUP
									// hangup, no answer
									if (theReason == "1")
									{
										theDispo = "-2";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
										TheQueues.rWhere(theCampaign).IncrementNoanswers();
									}
									// AST_CONTROL_CONGESTION
									// congestion
									if (theReason == "8")
									{
										theDispo = "-5";
										writeDBString(theCampaign, theLeadid, "disposition=(((disposition=-5)*-2)-5)");
										TheQueues.rWhere(theCampaign).IncrementCongestions();
									}
									if (theDispo == "0")
									{
										if (doColorize)
										{
											std::cout << theCampaign << fg_light_red << ": OriginateFailure - " << theReason << " (" << dispo2long(std::stoi(theDispo)) << ") " << normal << std::endl;
										}
										else
										{
											std::cout << theCampaign << ": OriginateFailure - " << theReason << " (" << dispo2long(std::stoi(theDispo)) << ") " << std::endl;
										}
									}
									if (gDebug)
									{
										if (doColorize)
										{
											std::cout << theCampaign << fg_light_green << ": OriginateFailure - theLeadid: " << theLeadid << " theReason: " << theReason << " theDispo: " << theDispo << " (" << dispo2long(std::stoi(theDispo)) << ") " << normal << std::endl;
										}
										else
										{
											std::cout << theCampaign << ": OriginateFailure - theLeadid: " << theLeadid << " theReason: " << theReason << " theDispo: " << theDispo << " (" << dispo2long(std::stoi(theDispo)) << ") " << std::endl;
										}
									}
									if (gLog)
									{
										writeGnudialerLog(theCampaign + ": OriginateFailure - theLeadid: " + theLeadid + " theReason: " + theReason + " theDispo: " + theDispo);
									}
								}
								else
								{
									if (doColorize)
									{
										std::cout << fg_light_red << "OriginateFailure: PARSE ERROR " << normal << std::endl;
									}
									else
									{
										std::cout << "OriginateFailure: PARSE ERROR " << std::endl;
									}
									if (gLog)
									{
										writeGnudialerLog("OriginateFailure - PARSE ERROR - Something Empty (block: " + block + ")");
									}
								}
							}
							else
							{
								if (gLog)
								{
									writeGnudialerLog("OriginateFailure - PARSE ERROR - No CallerIDName or Empty Reason (block: " + block + ")");
								}
							}
						}

						//***********************************************************************************
						if (block.find("Response: Success", 0) != std::string::npos)
						{
							// if (block.find("Event: OriginateSuccess",0) != std::string::npos) {
							int iTheReason = 0;
							std::string theReason, theCallerIDName, theCampaign, theLeadid;
							std::string theDispo = "-3";
							if (block.find("Reason: ", 0) != std::string::npos)
							{
								pos = block.find("Reason: ", 0) + 8;
								end = block.find("\n", pos);
								theReason = block.substr(pos, end - pos);
								iTheReason = atoi(theReason.c_str());
							}
							if (block.find("CallerIDName: ", 0) != std::string::npos && block.find("~", 0) != std::string::npos && !theReason.empty())
							{
								pos = block.find("CallerIDName: ", 0) + 15;
								end = block.find("\n", pos);
								theCallerIDName = block.substr(pos, end - pos);

								if (block.find("~", 0) != std::string::npos)
								{
									pos = theCallerIDName.find("~", end) + 1;
									end = theCallerIDName.find("-", pos + 1);

									pos2 = end + 1;
									end2 = theCallerIDName.find("-", pos2);

									theCampaign = theCallerIDName.substr(pos, end - pos);
									theLeadid = theCallerIDName.substr(pos2, end2 - pos2);
								}

								if (!theCampaign.empty() && !theLeadid.empty() && TheQueues.exists(theCampaign))
								{
									// answer, we'll assume voicemail/answering machine
									// if it passes talkdetect it will be sent to agent
									// or abandons++
									if (theReason == "4")
									{
										theDispo = "-3";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
										TheQueues.rWhere(theCampaign).IncrementAnsmachs();
									}
									// timed out while ringing, no answer
									if (theReason == "3")
									{
										theDispo = "-2";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
									}
									// busy
									if (theReason == "5")
									{
										theDispo = "-4";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
									}
									// hangup, no answer
									if (theReason == "1")
									{
										theDispo = "-2";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
									}
									// congestion
									if (theReason == "8")
									{
										theDispo = "-5";
										writeDBString(theCampaign, theLeadid, "disposition='" + theDispo + "'");
									}
									if (theDispo == "0")
									{
										if (doColorize)
										{
											std::cout << theCampaign << fg_light_red << ": OriginateSuccess - UNKNOWN REASON - " << theReason << normal << std::endl;
										}
										else
										{
											std::cout << theCampaign << ": OriginateSuccess - UNKNOWN REASON - " << theReason << std::endl;
										}
										if (gLog)
										{
											writeGnudialerLog(theCampaign + ": OriginateSuccess - UNKNOWN REASON - theLeadid: " + theLeadid + " theReason: " + theReason + " theDispo: " + theDispo);
										}
									}
									if (gDebug)
									{
										if (doColorize)
										{
											std::cout << theCampaign << fg_green << ": OriginateSuccess - theLeadid: " << theLeadid << " theReason: " << theReason << " theDispo: " << theDispo << " (" << dispo2long(std::stoi(theDispo)) << ") " << normal << std::endl;
										}
										else
										{
											std::cout << theCampaign << ": OriginateSuccess - theLeadid: " << theLeadid << " theReason: " << theReason << " theDispo: " << theDispo << " (" << dispo2long(std::stoi(theDispo)) << ") " << std::endl;
										}
									}
									if (gLog)
									{
										writeGnudialerLog(theCampaign + ": OriginateSuccess - theLeadid: " + theLeadid + " theReason: " + theReason + " theDispo: " + theDispo);
									}
								}
								else
								{
									if (doColorize)
									{
										std::cout << fg_light_red << "OriginateSuccess: PARSE ERROR " << normal << std::endl;
									}
									else
									{
										std::cout << "OriginateSuccess: PARSE ERROR " << std::endl;
									}
									if (gLog)
									{
										writeGnudialerLog("OriginateSuccess - PARSE ERROR - Something Empty (block: " + block + ")");
									}
								}
							}
							else
							{
								if (gLog)
								{
									writeGnudialerLog("OriginateSuccess - PARSE ERROR - No CallerIDName or Empty Reason (block: " + block + ")");
								}
							}
						}
					}
					//***********************************************************************************
					if (block.find("Event: Agentlogin", 0) != std::string::npos || block.find("Event: Agentcallbacklogin", 0) != std::string::npos)
					{
						if (block.find("Agent: ", 0) != std::string::npos)
						{
							pos = block.find("Agent: ", 0) + 7;
							end = block.find("\n", pos);

							std::string theAgent;
							theAgent = block.substr(pos, end - pos);
							if (gDebug)
							{
								std::cout << "theAgent: " << theAgent << " - login attempt" << std::endl;
							}

							if (TheAgents.exists(atoi(theAgent.c_str())))
							{
								if (gDebug)
								{
									if (doColorize)
									{
										std::cout << "theAgent: " << fg_light_green << theAgent << " - logged in" << normal << std::endl;
									}
									else
									{
										std::cout << "theAgent: " << theAgent << " - logged in" << std::endl;
									}
								}
								TheAgents.where(atoi(theAgent.c_str())).SetLoggedIn();
								if (gLog)
								{
									writeGnudialerLog("SetLoggedIn: " + theAgent);
								}
							}
							else
							{
								std::cerr << "AgentLogin: Error parsing agent number!" << std::endl;
							}
						}
					}

					//***********************************************************************************
					if (block.find("Event: Agentlogoff", 0) != std::string::npos || block.find("Event: Agentcallbacklogoff", 0) != std::string::npos)
					{
						if (block.find("Agent: ", 0) != std::string::npos)
						{
							pos = block.find("Agent: ", 0) + 7;
							end = block.find("\n", pos);

							std::string theAgent;
							theAgent = block.substr(pos, end - pos);

							if (gDebug)
							{
								std::cout << "theAgent: " << theAgent << " - logoff attempt" << std::endl;
							}

							if (TheAgents.exists(atoi(theAgent.c_str())))
							{
								TheAgents.where(atoi(theAgent.c_str())).SetOffline();
								if (gDebug)
								{
									if (doColorize)
									{
										std::cout << "theAgent: " << fg_light_green << theAgent << " - logged off" << normal << std::endl;
									}
									else
									{
										std::cout << "theAgent: " << theAgent << " - logged off" << std::endl;
									}
								}
								if (gLog)
								{
									writeGnudialerLog("SetOffline: " + theAgent);
								}
							}
							else
							{
								std::cerr << "AgentLogoff: Error parsing agent number!" << std::endl;
							}
						}
					}

					//***********************************************************************************
					if (block.find("Event: REMOVEFOROLDUSAGEUnlink", 0) != std::string::npos &&
						block.find("PJSIP/", 0) != std::string::npos)
					{
						pos = block.find("PJSIP/", 0) + 6;
						end = block.find("\n", pos);

						std::string theAgent, theCampaign;
						theAgent = block.substr(pos, end - pos);

						if (gDebug)
						{
							std::cout << "theAgent: " << theAgent << " - unlink attempt" << std::endl;
						}

						if (TheAgents.exists(atoi(theAgent.c_str())))
						{
							theCampaign = TheAgents.where(atoi(theAgent.c_str())).GetCampaign();

							if (gDebug)
							{
								std::cout << theCampaign << ": theAgent - " << theAgent << " - unlinked" << std::endl;
							}

							if (theCampaign == "initialized")
							{
								TheAgents.where(atoi(theAgent.c_str())).SetOnWait(false, false, TheAgents);
								if (gLog)
								{
									writeGnudialerLog("SetOnWait: " + theAgent);
								}
							}
							else
							{
								// TheQueues.where(TheAgents.where(atoi(theAgent.c_str())).GetCampaign()).AddTalkTime(TheAgents.where(atoi(theAgent.c_str())).SetOnWait(true,true,TheAgents));
								TheQueues.where(TheAgents.where(atoi(theAgent.c_str())).GetCampaign()).AddTalkTime(TheAgents.where(atoi(theAgent.c_str())).SetOnWait(false, false, TheAgents));
								TheAgents.where(atoi(theAgent.c_str())).writeAgentLog(TheAgents);
								if (gLog)
								{
									writeGnudialerLog("SetOnWait: " + theAgent);
								}
							}
						}
						else
						{
							std::cout << "Unlink: Error parsing agent number!" << std::endl;
						}
					}

					//***********************************************************************************
					if (block.find("Event: Link", 0) != std::string::npos &&
						block.find("PJSIP/", 0) != std::string::npos)
					{
						pos = block.find("PJSIP/", 0) + 6;
						end = block.find("\n", pos);

						std::string theAgent;
						theAgent = block.substr(pos, end - pos);

						if (gDebug)
						{
							std::cout << "theAgent: " << theAgent << " - link attempt" << std::endl;
						}

						if (TheAgents.exists(atoi(theAgent.c_str())))
						{
							TheAgents.where(atoi(theAgent.c_str())).SetOnCall();

							if (gDebug)
							{
								std::cout << "theAgent: " << theAgent << " - linked" << std::endl;
							}
							if (gLog)
							{
								writeGnudialerLog("SetOnCall: " + theAgent);
							}
						}
						else
						{
							std::cout << "Link: Error parsing agent number!" << std::endl;
						}
					}

					//***********************************************************************************
					if (block.find("Event: UserEvent", 0) != std::string::npos)
					{
						//***********************************************************************************
						// if (block.find("Event: ManagerUserEvent", 0) != std::string::npos &&
						//		block.find("Event: CRM_DispoRecord", 0) != std::string::npos ||
						if (block.find("UserEvent: UserEventDispo", 0) != std::string::npos)
						{

							std::string theEvent, theAgent, theDispo, theChannel, theCampaign, theLeadid;
							std::string theTransfer, theAgentCloser, theDispoColumn, theCloserCam;
							bool tempUseCloser, tempPrintAgentSales, tempPrintCloserSales, tempPrintCloserNoSales;
							std::string tempCloserCam, theTempCampaign;

							theEvent = "DispoRecord";

							std::string tempStringAgent;
							int tempIntAgent;

							std::cout << "Manager/UserEvent - DispoRecord ";

							if (block.find("Agent: ") != std::string::npos)
							{
								pos = block.find("Agent: ") + 7;
								end = block.find("\n", pos);
								theAgent = block.substr(pos, end - pos);
								theAgent = trim(theAgent);
								if (gLog)
								{
									writeGnudialerLog("Asterisk: ManagerUserEvent - " + theEvent + " - " + theAgent + "");
								}

								std::cout << " Agent: " << theAgent;
							}

							if (block.find("Dispo: ") != std::string::npos)
							{
								pos = block.find("Dispo: ") + 7;
								end = block.find("\n", pos);
								theDispo = block.substr(pos, end - pos);

								std::cout << " Dispo: " << theDispo;
							}

							if (block.find("Transfer: ") != std::string::npos)
							{
								pos = block.find("Transfer: ") + 10;
								end = block.find("\n", pos);
								theTransfer = block.substr(pos, end - pos);

								std::cout << " Transfer: " << theTransfer;
							}

							if (block.find("Campaign: ") != std::string::npos)
							{
								pos = block.find("Campaign: ") + 10;
								end = block.find("\n", pos);
								theCampaign = block.substr(pos, end - pos);
								theCampaign = trim(theCampaign);
								// this is due to crm adding -isclosercam to campaign name
								if (theCampaign.find("-isclosercam", 0) != std::string::npos)
								{
									end2 = theCampaign.find("-", 0);
									theTempCampaign = theCampaign.substr(0, end2);
									theCampaign = theTempCampaign;
								}

								std::cout << " Campaign: ~" << theCampaign << "~";
							}

							if (block.find("Leadid: ") != std::string::npos)
							{
								pos = block.find("Leadid: ", 0) + 8;
								end = block.find("\n", pos);
								theLeadid = block.substr(pos, end - pos);

								std::cout << " Leadid: " << theLeadid;
							}

							if (block.find("Channel: ", 0) != std::string::npos)
							{
								pos = block.find("Channel: ", 0) + 9;
								end = block.find("\n", pos);
								theChannel = block.substr(pos, end - pos);

								std::cout << " Channel: " << theChannel << std::endl;
							}

							if (TheQueues.exists(theCampaign))
							{

								if (theTransfer == "TRANSFER")
								{
									try
									{
										tempUseCloser = TheQueues.rWhere(theCampaign).GetSetting("usecloser").GetBool();
									}
									catch (xLoopEnd e)
									{
										std::cout << "Caught xLoopEnd while trying to get usecloser variable" << std::endl;
										std::cout << e.what();
										std::cout << std::endl
												  << std::endl;
									}

									std::cout << " tempUseCloser: " << tempUseCloser;

									tempCloserCam = TheQueues.rWhere(theCampaign).GetSetting("closercam").Get();

									std::cout << " tempCloserCam: " << tempCloserCam;
								}

								std::cout << std::endl;

								if (TheAgents.exists(atoi(theAgent.c_str())))
								{
									tempIntAgent = atoi(theAgent.c_str());
									int tempAgentStatus = TheAgents.where(atoi(theAgent.c_str())).GetStatus();

									if (tempAgentStatus != -4 && tempAgentStatus != -3)
									{
										// TheQueues.where(TheAgents.where(atoi(theAgent.c_str())).GetCampaign()).AddTalkTime(TheAgents.where(atoi(theAgent.c_str())).SetOnWait(false,false,TheAgents));
										// theCampaign is passed by the call, do NOT use GetCampaign
										TheQueues.where(theCampaign).AddTalkTime(tempIntAgent);
										TheAgents.where(tempIntAgent).writeAgentLog(TheAgents);
									}
									TheAgents.where(tempIntAgent).SetOnWait(false, false, TheAgents);

									std::cout << "GnuDialer: SetOnWait - " << theAgent << std::endl;

									if (gLog)
									{
										writeGnudialerLog("GnuDialer: SetOnWait - " + theAgent + "");
									}
								}
								else
								{
									std::cerr << "DispoRecord: Error parsing agent number!" << std::endl;
								}

								// do not IGNore, we want the core
								// signal(SIGCLD, SIG_IGN);

								if (theTransfer == "TRANSFER")
								{
									theDispoColumn = "disposition";
									theAgentCloser = "agent";
								}
								else
								{
									theDispoColumn = "closerdispo";
									theAgentCloser = "closer";
								}

								writeDBString(theCampaign, theLeadid, "" + theDispoColumn + "='" + theDispo + "'," + theAgentCloser + "='" + theAgent + "'");

								std::cout << theCampaign << ": writeDBString - DispoRecord - " << theAgentCloser + ": " << theAgent << " theDispo: " << theDispo << std::endl;
								writeDispo(theAgent, theCampaign, theDispo);

								std::cout << theCampaign << ": writeDispo - DispoRecord - theAgent: " << theAgent << " theDispo: " << theDispo << std::endl;
								try
								{
									tempPrintAgentSales = TheQueues.rWhere(theCampaign).GetSetting("prn_agent_sales").GetBool();
									tempPrintCloserSales = TheQueues.rWhere(theCampaign).GetSetting("prn_closer_sales").GetBool();
									tempPrintCloserNoSales = TheQueues.rWhere(theCampaign).GetSetting("prn_closer_nosales").GetBool();
								}
								catch (const std::runtime_error &e)
								{
									tempPrintAgentSales = false;
									tempPrintCloserSales = false;
									tempPrintCloserNoSales = false;
								}

								if (theTransfer == "TRANSFER" && theDispo == "12" && tempPrintAgentSales)
								{
									pid = fork();
									if (pid == 0)
									{
										doPrintSale("Agent SALE NON-Verified", theCampaign, theLeadid);
										exit(0);
									}
									if (pid == -1)
									{
										throw xForkError();
									}
								}

								if (theTransfer == "TRANSFER" && theDispo == "12" && tempUseCloser)
								{
									tempStringAgent = TheQueues.LeastRecent(tempCloserCam, TheAgents);

									if (atoi(tempStringAgent.c_str()))
									{

										std::cout << theCampaign << ": Transfer - tempStringAgent: " << tempStringAgent << std::endl;

										if (TheAgents.exists(atoi(tempStringAgent.c_str())))
										{
											TheAgents.where(atoi(tempStringAgent.c_str())).SetConnectedChannel(theChannel);
											// testing here								TheAgents.where(atoi(tempStringAgent.c_str())).SetCampaign(TheQueues.rWhere(theCampaign).GetSetting("closercam").Get());
											TheAgents.where(atoi(tempStringAgent.c_str())).SetCampaign(theCampaign);

											TheAgents.where(atoi(tempStringAgent.c_str())).SetLeadId(theLeadid);
											TheAgents.where(atoi(tempStringAgent.c_str())).SetOnWait(false, true, TheAgents);

											std::cout << theCampaign << ": theLeadid - " << theLeadid << std::endl;

											pid = fork();
											if (pid == 0)
											{
												doAriRedirect(theChannel, tempStringAgent, tempCloserCam, theLeadid, true, TheAgents);
												exit(0);
											}
											if (pid == -1)
											{
												throw xForkError();
											}
										}
										else
										{
											std::cerr << "DispoRecord: Closer no longer Exists (DISPO)!" << std::endl;
										}
									}
									else
									{
										std::cout << theCampaign << ": No available Closers! (DISPO)" << std::endl;

										pid = fork();
										if (pid == 0)
										{
											doAriRedirect(theChannel, "699", tempCloserCam, theLeadid, true, TheAgents);
											exit(0);
										}
										if (pid == -1)
										{
											throw xForkError();
										}
									}
								}
								else
								{

									if (theTransfer == "" && theDispo == "11" && tempPrintCloserNoSales)
									{
										pid = fork();
										if (pid == 0)
										{
											doPrintSale("SALE Verification FAILED (for some reason)", theCampaign, theLeadid);
											exit(0);
										}
										if (pid == -1)
										{
											throw xForkError();
										}
									}

									if (theDispo == "12" && tempPrintCloserSales)
									{
										pid = fork();
										if (pid == 0)
										{
											doPrintSale("Agent SALE (Verified/or not Required)", theCampaign, theLeadid);
											exit(0);
										}
										if (pid == -1)
										{
											throw xForkError();
										}
									}

									if (!theChannel.empty())
									{
										pid = fork();
										if (pid == 0)
										{
											doHangupCall(theChannel, theAgent, managerUser, managerPass);
											exit(0);
										}
										if (pid == -1)
										{
											throw xForkError();
										}
									}
									else
									{
										std::cerr << "DispoRecord: Channel Empty (HANGUP)!" << std::endl;
									}
								}
							}
							else
							{
								std::cout << theCampaign << ": Parse ERROR! (DISPO)" << std::endl;
							}
						}

						if (block.find("UserEvent: SetOnCall") != std::string::npos)
						{
							size_t agentPos = block.find("Agent: ");
							std::string agentID;
							if (agentPos != std::string::npos)
							{
								agentID = block.substr(agentPos + 7); // "Agent: " is 7 characters long
								std::cout << "Agent on call: " << agentID << std::endl;
								TheAgents.where(atoi(agentID.c_str())).SetOnCall();
							}
						}

						if (block.find("UserEvent: SetOnWait") != std::string::npos)
						{
							size_t agentPos = block.find("Agent: ");
							std::string agentID;
							if (agentPos != std::string::npos)
							{
								agentID = block.substr(agentPos + 7); // "Agent: " is 7 characters long
								agentID.erase(agentID.find_last_not_of(" \n\r\t") + 1);
								std::cout << "Agent on Wait: ~" << agentID << "~" << std::endl;
								TheAgents.where(atoi(agentID.c_str())).SetLoggedIn();
							}
						}
						if ((block.find("UserEvent: Queue|", 0) != std::string::npos && block.find("~", 0) != std::string::npos) ||
							(block.find("UserEvent: QueueTRANSFER", 0) != std::string::npos && block.find("~", 0) != std::string::npos))
						{

							bool isNone = false;
							std::string theChannel, theAgent, theLeadid, theCampaign;
							bool isTransfer = false;
							std::string tempQueueAgent, tempQueueCampaign;

							if (block.find("Channel: ", 0) != std::string::npos)
							{
								pos = block.find("Channel: ", 0) + 9;
								end = block.find("\n", pos);
								theChannel = block.substr(pos, end - pos);
								if (gDebug)
								{
									std::cout << "UserEventQueue: theChannel - " << theChannel << std::endl;
								}
							}
							if (block.find("~", 0) != std::string::npos)
							{
								pos = block.find("~", end) + 1;
								end = block.find("-", pos + 1);
								theCampaign = block.substr(pos, end - pos);
								if (theCampaign.find("none", 0) != std::string::npos)
								{
									isNone = true;
								}
								if (gDebug)
								{
									std::cout << "UserEventQueue: theCampaign - " << theCampaign << std::endl;
								}
								pos2 = end + 1;
								end2 = block.find("-", pos2);
								theLeadid = block.substr(pos2, end2 - pos2);
								if (gDebug)
								{
									std::cout << "UserEventQueue: theLeadid - " << theLeadid << std::endl;
								}
							}

							if (TheQueues.exists(theCampaign))
							{

								if (block.find("TRANSFER", 0) != std::string::npos && !isNone)
								{
									isTransfer = true;
									if (gDebug)
									{
										std::cout << "UserEventQueue: isTransfer - " << isTransfer << std::endl;
									}
									if (gDebug)
									{
										std::cout << theCampaign << ": Setting Answered - theLeadid: " << theLeadid << std::endl;
									}
									TheCallCache->SetAnswered(theCampaign, theLeadid);
								}

								// do not IGNore, we want the core
								// signal(SIGCLD, SIG_IGN);

								if (isTransfer)
								{
									//******************************************************************************
									// this section is where calls are handled for CLOSERS
									try
									{
										tempQueueAgent = TheQueues.LeastRecent(TheQueues.rWhere(theCampaign).GetSetting("closercam").Get(), TheAgents);
										tempQueueCampaign = TheQueues.rWhere(theCampaign).GetSetting("closercam").Get();
									}
									catch (xLoopEnd e)
									{
										std::cout << "Caught xLoopEnd when getting closercam." << std::endl;
										std::cout << e.what();
										std::cout << std::endl
												  << std::endl;
									}
									if (atoi(tempQueueAgent.c_str()))
									{
										if (gDebug)
										{
											std::cout << theCampaign << ": Transfer - tempQueueAgent: " << tempQueueAgent << std::endl;
										}

										if (isNone == false)
										{
											if (gDebug)
											{
												std::cout << theCampaign << ": Setting Answered - theLeadid: " << theLeadid << std::endl;
											}
											TheCallCache->SetAnswered(theCampaign, theLeadid);
										}

										if (TheAgents.exists(atoi(tempQueueAgent.c_str())))
										{
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetConnectedChannel(theChannel);
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetCampaign(theCampaign);
											// TheAgents.where(atoi(tempQueueAgent.c_str())).SetCampaign(TheQueues.rWhere(theCampaign).GetSetting("closercam").Get());
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetLeadId(theLeadid);
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetOnWait(false, false, TheAgents);
											// TheAgents.where(atoi(tempQueueAgent.c_str())).SetOnWait(false,true,TheAgents);

											if (gDebug)
											{
												std::cout << theCampaign << ": theLeadid - " << theLeadid << std::endl;
											}
										}

										pid = fork();
										if (pid == 0)
										{
											if (isNone == false)
											{
												doAriRedirect(theChannel, tempQueueAgent, theCampaign, theLeadid, true, TheAgents);
												exit(0);
											}
										}

										if (pid == -1)
										{
											throw xForkError();
										}
									}
									else
									{

										std::cout << theCampaign << ": No available Closers! (QUEUE)" << std::endl;

										pid = fork();
										if (pid == 0)
										{
											doAriRedirect(theChannel, "699", theCampaign, theLeadid, true, TheAgents);
											// doHangupCall(theChannel,theAgent,managerUser,managerPass);
											exit(0);
										}
										if (pid == -1)
										{
											throw xForkError();
										}
									}
								}
								else
								{
									//******************************************************************************
									// this section is where calls are handled for AGENTS
									tempQueueAgent = TheQueues.LeastRecent(theCampaign, TheAgents);
									if (atoi(tempQueueAgent.c_str()))
									{
										if (gDebug)
										{
											std::cout << theCampaign << ": Non-Transfer - tempQueueAgent: " << tempQueueAgent << std::endl;
										}

										if (isNone == false)
										{
											if (gDebug)
											{
												std::cout << theCampaign << ": Setting Answered - theLeadid: " << theLeadid << std::endl;
											}
											TheCallCache->SetAnswered(theCampaign, theLeadid);
											TheQueues.rWhere(theCampaign).DecrementAnsmachs();
										}

										if (TheAgents.exists(atoi(tempQueueAgent.c_str())))
										{
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetConnectedChannel(theChannel);
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetCampaign(theCampaign);
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetLeadId(theLeadid);
											TheAgents.where(atoi(tempQueueAgent.c_str())).SetOnWait(false, false, TheAgents);
											if (gDebug)
											{
												std::cout << theCampaign << ": theLeadid - " << theLeadid << std::endl;
											}
										}

										pid = fork();
										if (pid == 0)
										{
											if (isNone == false)
											{
												doAriRedirect(theChannel, tempQueueAgent, theCampaign, theLeadid, false, TheAgents);
												exit(0);
											}
										}
										if (pid == -1)
										{
											throw xForkError();
										}
									}
									else
									{
										std::cout << theCampaign << ": No available Agents! (QUEUE)" << std::endl;
									}
								}
							}
							else
							{
								std::cout << theCampaign << ": Parse ERROR! (QUEUE)" << std::endl;
							}
						}

						if (block.find("UserEvent: Abandon", 0) != std::string::npos)
						{
							std::string theCallerIDName, theCampaign, theAgent, theLeadid;
							if (gDebug)
							{
								std::cout << "UserEvent - Abandon ";
							}

							if (block.find("CallerIDName: ", 0) != std::string::npos && block.find("~", 0) != std::string::npos)
							{

								pos = block.find("CallerIDName: ", 0) + 15;
								end = block.find("\n", pos);
								theCallerIDName = block.substr(pos, end - pos);

								pos = theCallerIDName.find("~", end) + 1;
								end = theCallerIDName.find("-", pos + 1);

								pos2 = end + 1;
								end2 = theCallerIDName.find("-", pos2);

								theCampaign = theCallerIDName.substr(pos, end - pos);
								if (gDebug)
								{
									std::cout << " theCampaign: " << theCampaign;
								}

								theLeadid = theCallerIDName.substr(pos2, end2 - pos2);
								if (gDebug)
								{
									std::cout << " theLeadid: " << theLeadid;
								}

								if (gDebug)
								{
									std::cout << std::endl;
								}

								if (!theCampaign.empty() && !theLeadid.empty() && TheQueues.exists(theCampaign))
								{
									TheQueues.rWhere(theCampaign).IncrementAbandons();
									TheQueues.rWhere(theCampaign).WriteAbn();
									TheQueues.rWhere(theCampaign).WriteCalls();

									writeDBString(theCampaign, theLeadid, "abandons=abandons+1");
									if (gDebug)
									{
										std::cout << theCampaign << ": writeDBString - Abandon " << std::endl;
									}
									if (gLog)
									{
										writeGnudialerLog(theCampaign + ": theLeadid - " + theLeadid + " was abanadoned");
									}
								}
								else
								{
									if (gDebug)
									{
										std::cout << "UserEventAbandon: Parse ERROR " << std::endl;
									}
								}
							}
						}

						if (block.find("UserEvent: Pickup", 0) != std::string::npos)
						{
							std::string theCallerIDName, theCampaign, theAgent, theLeadid;
							if (gDebug)
							{
								std::cout << "UserEvent - Pickup ";
							}

							if (block.find("CallerIDName: ", 0) != std::string::npos && block.find("~", 0) != std::string::npos)
							{
								pos = block.find("CallerIDName: ", 0) + 15;
								end = block.find("\n", pos);
								theCallerIDName = block.substr(pos, end - pos);

								pos = theCallerIDName.find("~", end) + 1;
								end = theCallerIDName.find("-", pos + 1);

								pos2 = end + 1;
								end2 = theCallerIDName.find("-", pos2);

								theCampaign = theCallerIDName.substr(pos, end - pos);
								if (gDebug)
								{
									std::cout << " theCampaign: " << theCampaign;
								}

								theLeadid = theCallerIDName.substr(pos2, end2 - pos2);
								if (gDebug)
								{
									std::cout << " theLeadid: " << theLeadid;
								}

								if (gDebug)
								{
									std::cout << std::endl;
								}

								if (!theCampaign.empty() && !theLeadid.empty() && TheQueues.exists(theCampaign))
								{
									writeDBString(theCampaign, theLeadid, "pickups=pickups+1");
									if (gDebug)
									{
										std::cout << theCampaign << ": writeDBString - Pickup " << std::endl;
									}
									if (gLog)
									{
										writeGnudialerLog(theCampaign + ": theLeadid - " + theLeadid + " was picked-up");
									}
								}
								else
								{
									if (gDebug)
									{
										std::cout << "UserEventPickup: Parse ERROR " << std::endl;
									}
								}
							}
						}

						if (block.find("UserEvent: Fax", 0) != std::string::npos)
						{
							std::string theCallerIDName, theCampaign, theAgent, theLeadid;
							if (gDebug)
							{
								std::cout << "UserEvent - Fax ";
							}

							if (block.find("CallerIDName: ", 0) != std::string::npos && block.find("~", 0) != std::string::npos)
							{

								pos = block.find("CallerIDName: ", 0) + 15;
								end = block.find("\n", pos);
								theCallerIDName = block.substr(pos, end - pos);

								pos = theCallerIDName.find("~", end) + 1;
								end = theCallerIDName.find("-", pos + 1);

								pos2 = end + 1;
								end2 = theCallerIDName.find("-", pos2);

								theCampaign = theCallerIDName.substr(pos, end - pos);
								if (gDebug)
								{
									std::cout << " theCampaign: " << theCampaign;
								}

								theLeadid = theCallerIDName.substr(pos2, end2 - pos2);
								if (gDebug)
								{
									std::cout << " theLeadid: " << theLeadid;
								}

								if (gDebug)
								{
									std::cout << std::endl;
								}
								writeDBString(theCampaign, theLeadid, "disposition='-6',pickups=pickups+1");
								if (gDebug)
								{
									std::cout << theCampaign << ": writeDBString - Fax " << std::endl;
								}
							}
						}
						// END USER_EVENTS
					}

					//***********************************************************************************
					if (block.find("Event: ManagerUserEvent", 0) != std::string::npos &&
						block.find("Event: AgentPause", 0) != std::string::npos)
					{

						std::string theEvent, theAgent;
						theEvent = "Pause";

						if (block.find("Agent: ", 0) != std::string::npos)
						{
							pos = block.find("Agent: ", 0) + 7;
							end = block.find("\n", pos);
							theAgent = block.substr(pos, end - pos);
						}

						if (gDebug)
						{
							std::cout << "Asterisk: ManagerUserEvent - " << theEvent << " - " << theAgent << std::endl;
						}
						if (gLog)
						{
							writeGnudialerLog("Asterisk: ManagerUserEvent - " + theEvent + " - " + theAgent + "");
						}

						if (TheAgents.exists(atoi(theAgent.c_str())))
						{
							TheAgents.where(atoi(theAgent.c_str())).SetOnPause();
							if (gDebug)
							{
								std::cout << "GnuDialer: SetOnPause - " << theAgent << std::endl;
							}
							if (gLog)
							{
								writeGnudialerLog("GnuDialer: SetOnPause - " + theAgent + "");
							}
						}
						else
						{
							std::cerr << "AgentPause: Error parsing agent number!" << std::endl;
						}
					}

					//***********************************************************************************
					if (block.find("Event: ManagerUserEvent", 0) != std::string::npos &&
						block.find("Event: AgentUnPause", 0) != std::string::npos)
					{

						std::string theEvent, theAgent;
						theEvent = "UnPause";

						if (block.find("Agent: ", 0) != std::string::npos)
						{
							pos = block.find("Agent: ", 0) + 7;
							end = block.find("\n", pos);
							theAgent = block.substr(pos, end - pos);
						}

						if (gDebug)
						{
							std::cout << "Asterisk: ManagerUserEvent - " << theEvent << " - " << theAgent << std::endl;
						}
						if (gLog)
						{
							writeGnudialerLog("Asterisk: ManagerUserEvent - " + theEvent + " - " + theAgent + "");
						}

						if (TheAgents.exists(atoi(theAgent.c_str())))
						{
							TheAgents.where(atoi(theAgent.c_str())).SetOnWait(false, false, TheAgents);
							if (gDebug)
							{
								std::cout << "GnuDialer: SetOnWait - " << theAgent << std::endl;
							}
							if (gLog)
							{
								writeGnudialerLog("GnuDialer: SetOnWait - " + theAgent + "");
							}
						}
						else
						{
							std::cerr << "AgentUnPause: Error parsing agent number!" << std::endl;
						}
					}

					//***********************************************************************************
					if (block.find("Event: ManagerUserEvent", 0) != std::string::npos &&
						block.find("Event: AgentMonitor", 0) != std::string::npos)
					{

						std::string theEvent, theAgent, theCampaign, theLeadid, theChannel, theTempCampaign;

						theEvent = "Monitor";

						if (gDebug)
						{
							std::cout << "ManagerUserEvent - " + theEvent + " - ";
						}

						if (block.find("Agent: ", 0) != std::string::npos)
						{
							pos = block.find("Agent: ", 0) + 7;
							end = block.find("|", pos);
							theAgent = block.substr(pos, end - pos);
							if (gLog)
							{
								writeGnudialerLog("ManagerUserEvent - " + theEvent + " - " + theAgent + "");
							}
							if (gDebug)
							{
								std::cout << " Agent: " << theAgent;
							}
						}

						if (block.find("Campaign: ", 0) != std::string::npos)
						{
							pos = block.find("Campaign: ", 0) + 10;
							end = block.find("|", pos);
							theCampaign = block.substr(pos, end - pos);
							// this is due to crm adding -isclosercam to campaign name
							if (theCampaign.find("-isclosercam", 0) != std::string::npos)
							{
								end2 = theCampaign.find("-", 0);
								theTempCampaign = theCampaign.substr(0, end2);
								theCampaign = theTempCampaign;
							}
							if (gDebug)
							{
								std::cout << " Campaign: " << theCampaign;
							}
						}

						if (block.find("Leadid: ", 0) != std::string::npos)
						{
							pos = block.find("Leadid: ", 0) + 8;
							end = block.find("|", pos);
							theLeadid = block.substr(pos, end - pos);
							if (gDebug)
							{
								std::cout << " Leadid: " << theLeadid;
							}
						}

						if (block.find("Channel: ", 0) != std::string::npos)
						{
							pos = block.find("Channel: ", 0) + 9;
							end = block.find("\n", pos);
							theChannel = block.substr(pos, end - pos);
							// if (gDebug) {
							//         std::cout << " Channel: " << theChannel;
							// }
						}

						if (gDebug)
						{
							std::cout << std::endl;
						}

						if (gLog)
						{
							writeGnudialerLog("Asterisk: ManagerUserEvent - " + theEvent + " - " + theAgent + "");
						}

						pid = fork();
						if (pid == 0)
						{
							doMonitorStart(theChannel, theAgent, theCampaign, theLeadid, managerUser, managerPass);
							exit(0);
						}
						if (pid == -1)
						{
							throw xForkError();
						}
					}

					//***********************************************************************************
					if (block.find("Event: ManagerUserEvent", 0) != std::string::npos &&
						block.find("Event: AgentUnMonitor", 0) != std::string::npos)
					{

						std::string theEvent, theAgent, theCampaign, theLeadid, theChannel, theTempCampaign;

						theEvent = "UnMonitor";

						if (gDebug)
						{
							std::cout << "ManagerUserEvent - " + theEvent + " - ";
						}

						if (block.find("Agent: ", 0) != std::string::npos)
						{
							pos = block.find("Agent: ", 0) + 7;
							end = block.find("|", pos);
							theAgent = block.substr(pos, end - pos);
							if (gLog)
							{
								writeGnudialerLog("ManagerUserEvent - " + theEvent + " - " + theAgent + "");
							}
							if (gDebug)
							{
								std::cout << " Agent: " << theAgent;
							}
						}

						if (block.find("Campaign: ", 0) != std::string::npos)
						{
							pos = block.find("Campaign: ", 0) + 10;
							end = block.find("|", pos);
							theCampaign = block.substr(pos, end - pos);
							// this is due to crm adding -isclosercam to campaign name
							if (theCampaign.find("-isclosercam", 0) != std::string::npos)
							{
								end2 = theCampaign.find("-", 0);
								theTempCampaign = theCampaign.substr(0, end2);
								theCampaign = theTempCampaign;
							}
							if (gDebug)
							{
								std::cout << " Campaign: " << theCampaign;
							}
						}

						if (block.find("Leadid: ", 0) != std::string::npos)
						{
							pos = block.find("Leadid: ", 0) + 8;
							end = block.find("|", pos);
							theLeadid = block.substr(pos, end - pos);
							if (gDebug)
							{
								std::cout << " Leadid: " << theLeadid;
							}
						}

						if (block.find("Channel: ", 0) != std::string::npos)
						{
							pos = block.find("Channel: ", 0) + 9;
							end = block.find("\n", pos);
							theChannel = block.substr(pos, end - pos);
							// if (gDebug) {
							//         std::cout << " Channel: " << theChannel;
							// }
						}

						if (gDebug)
						{
							std::cout << std::endl;
						}

						if (gLog)
						{
							writeGnudialerLog("Asterisk: ManagerUserEvent - " + theEvent + " - " + theAgent + "");
						}

						pid = fork();
						if (pid == 0)
						{
							doMonitorStop(theChannel, theAgent, theCampaign, theLeadid, managerUser, managerPass);
							exit(0);
						}
						if (pid == -1)
						{
							throw xForkError();
						}
					}

					//***********************************************************************************
					// this isn't used anymore, just left as a template incase we find something we want to
					// trigger off the Unlink event
					//
					//  This block is to make sure that just in case an Unlink event fails
					// we still set the agent on wait so that they still get calls!
					// if (block.find("Event: Hangup",0) != std::string::npos && block.find("Channel: ",0) != std::string::npos) {
					//      pos = block.find("Channel: ",0) + 9;
					//      end = block.find("\n",pos);
					//      std::string theChannel, theAgent;
					//      theChannel = block.substr(pos,end-pos);

					//	if (gDebug) {
					//        	std::cout << "theChannel: " << theChannel << " - hungup" << std::endl;
					//	}
					//	if (TheAgents.existsConnected(theChannel) && block.find("PJSIP/",0) != std::string::npos) {
					//        	TheAgents.whereConnected(theChannel).SetOnWait(false,false,TheAgents);
					//        	theAgent = TheAgents.whereConnected(theChannel).GetNumber();
					//        	if (gDebug) {
					//                	std::cout << "theAgent: " << theAgent << " - on wait" << std::endl;
					//        	}
					//        	if (gLog) {
					//                	writeGnudialerLog("SetOnWait: " + theChannel);
					//        	}
					//	}
					//}

					//***********************************************************************************
					if (block.find("Event: ManagerUserEvent", 0) != std::string::npos &&
						block.find("Event: CRM_CallHangup", 0) != std::string::npos)
					{

						std::string theEvent, theAgent, theChannel, theCampaign, theLeadid, theTempCampaign;
						theEvent = "CallHangup";
						if (gDebug)
						{
							std::cout << "UserEvent - CallHangup ";
						}

						//
						if (block.find("Agent: ", 0) != std::string::npos)
						{
							pos = block.find("Agent: ", 0) + 7;
							end = block.find("|", pos);
							theAgent = block.substr(pos, end - pos);
							if (gLog)
							{
								writeGnudialerLog("ManagerUserEvent - " + theEvent + " - " + theAgent + "");
							}
							if (gDebug)
							{
								std::cout << " Agent: " << theAgent;
							}
						}

						if (block.find("Campaign: ", 0) != std::string::npos)
						{
							pos = block.find("Campaign: ", 0) + 10;
							end = block.find("|", pos);
							theCampaign = block.substr(pos, end - pos);
							// this is due to crm adding -isclosercam to campaign name
							if (theCampaign.find("-isclosercam", 0) != std::string::npos)
							{
								end2 = theCampaign.find("-", 0);
								theTempCampaign = theCampaign.substr(0, end2);
								theCampaign = theTempCampaign;
							}
							if (gDebug)
							{
								std::cout << " Campaign: " << theCampaign;
							}
						}

						if (block.find("Leadid: ", 0) != std::string::npos)
						{
							pos = block.find("Leadid: ", 0) + 8;
							end = block.find("|", pos);
							theLeadid = block.substr(pos, end - pos);
							if (gDebug)
							{
								std::cout << " Leadid: " << theLeadid;
							}
						}

						if (block.find("Channel: ", 0) != std::string::npos)
						{
							pos = block.find("Channel: ", 0) + 9;
							end = block.find("\n", pos);
							theChannel = block.substr(pos, end - pos);
							if (gDebug)
							{
								std::cout << " Channel: " << theChannel;
							}
						}

						if (TheAgents.exists(atoi(theAgent.c_str())))
						{
							std::cout << " (agent exists) ";

							if (TheAgents.where(atoi(theAgent.c_str())).GetStatus() != -4)
							{
								std::cout << " (GetStatus ne -4) ";
								// crm is sending the campaign info back now
								// std::string itsCampaign = TheAgents.where(atoi(theAgent.c_str())).GetCampaign();
								if (!theCampaign.empty() && theCampaign != "initialized")
								{
									std::cout << " (campaign not empty, nor equal initialized) ";
									TheQueues.where(theCampaign).AddTalkTime(TheAgents.where(atoi(theAgent.c_str())).SetOnWait(false, false, TheAgents));
									// TheQueues.where(TheAgents.where(atoi(theAgent.c_str())).GetCampaign()).AddTalkTime(TheAgents.where(atoi(theAgent.c_str())).SetOnWait(false,false,TheAgents));
									std::cout << " (added talktime) ";
									TheAgents.where(atoi(theAgent.c_str())).writeAgentLog(TheAgents);
									std::cout << " (writeAgentLog) ";
									TheAgents.where(atoi(theAgent.c_str())).SetInCloseout();
									std::cout << std::endl;
									if (gDebug)
									{
										std::cout << theCampaign << ": SetInCloseout - " << theAgent << std::endl;
									}
									if (gLog)
									{
										writeGnudialerLog(theCampaign + ": SetInCloseout - " + theAgent + "");
									}
								}
								else
								{
									std::cout << theAgent << ": UserEvent - CallHangup - ERROR: Empty Campaign or 'initialized'" << std::endl;
								}
							}
						}
						else
						{
							std::cerr << "CallHangup: Error parsing agent number! (CLOSEOUT)" << std::endl;
						}

						// do not IGNore, we want the core
						// signal(SIGCLD, SIG_IGN);

						// passed by crm now
						// theChannel = TheAgents.where(atoi(theAgent.c_str())).GetConnectedChannel();
						if (!theChannel.empty())
						{

							pid = fork();
							if (pid == 0)
							{
								doHangupCall(theChannel, theAgent, managerUser, managerPass);
								exit(0);
							}
							if (pid == -1)
							{
								throw xForkError();
							}
						}
						else
						{
							std::cout << theAgent << ": UserEvent - CallHangup - ERROR: Empty Channel " << std::endl;
						}
					}

					//***********************************************************************************
					//        	                if (block.find("Event: UserEventPickup",0) != std::string::npos) {
					//                	                std::string theCallerIDName, theCampaign, theAgent, theLeadid;
					//                        	        if (gDebug) {
					//	        	        		std::cout << "UserEvent - Pickup ";
					//					}
					//
					//                                	if (block.find("CallerIDName: ",0) != std::string::npos && block.find("~",0) != std::string::npos) {
					//
					//                                        	pos = block.find("CallerIDName: ",0) + 15;
					//                                        	end = block.find("\n",pos);
					//                                        	theCallerIDName = block.substr(pos,end-pos);
					//
					//	                                        pos = theCallerIDName.find("~",end) + 1;
					//        	                                end = theCallerIDName.find("-",pos+1);
					//
					//                	                        pos2 = end + 1;
					//                        	                end2 = theCallerIDName.find("-",pos2);
					//
					//                                	        theCampaign = theCallerIDName.substr(pos,end-pos);
					//	                                	if (gDebug) {
					//		        	        		std::cout << " theCampaign: " << theCampaign;
					//						}
					//
					//	                                        theLeadid   = theCallerIDName.substr(pos2,end2-pos2);
					//        	                	        if (gDebug) {
					//	        		        		std::cout << " theLeadid: " << theLeadid;
					//						}
					//
					//        	                                if (gDebug) {
					//                	                                std::cout << std::endl;
					//                        	                }
					//						writeDBString(theCampaign,theLeadid,"pickups=pickups+1");
					//                                                if (gDebug) {
					//                                                        std::cout << theCampaign << ": writeDBString - Pickup " << std::endl;
					//                                                }
					//					}
					//				}

					//***********************************************************************************

					//***********************************************************************************
					// End block analysis
					block = "";
				}
				else
				{
					block += tempLine + "\n";
				}
			}

			// DIALING LOOP
			for (int i = 0; i < TheQueues.size(); i++)
			{
				if (TheQueues.at(i).GetSetting("active").Get() == "true")
				{
					queue = TheQueues.at(i).GetName();

					maxratio = TheQueues.at(i).GetSetting("maxratio").GetFloat();
					maxlines = TheQueues.at(i).GetSetting("maxlines").GetInt();
					maxabandons = TheQueues.at(i).GetSetting("maxabandons").GetFloat();
					mode = TheQueues.at(i).GetSetting("function").Get();
					calltoday = TheQueues.at(i).GetSetting("calltoday").Get();
					usednc = TheQueues.at(i).GetSetting("usednc").Get();
					callerid = TheQueues.at(i).GetSetting("callerid").Get();
					filter = TheQueues.at(i).GetSetting("filter").Get();
					timeout = TheQueues.at(i).GetSetting("timeout").GetInt();
					usecloser = TheQueues.at(i).GetSetting("usecloser").Get();
					closercam = TheQueues.at(i).GetSetting("closercam").Get();
					dspmode = TheQueues.at(i).GetSetting("dspmode").Get();
					trunk = TheQueues.at(i).GetSetting("trunk").Get();
					dialprefix = TheQueues.at(i).GetSetting("dialprefix").Get();
					usecallback = TheQueues.at(i).GetSetting("usecallback").Get();
					usetzfilter = TheQueues.at(i).GetSetting("usetzfilter").Get();
					debug = TheQueues.at(i).GetSetting("debug").GetBool();
					skip = TheQueues.at(i).GetSetting("skip").GetInt();
					f_areacode = TheQueues.at(i).GetSetting("f_areacode").Get();
					f_areacode_prefix = TheQueues.at(i).GetSetting("f_areacode_prefix").Get();
					f_zipcode = TheQueues.at(i).GetSetting("f_zipcode").Get();
					orderby = TheQueues.at(i).GetSetting("orderby").Get();

					// put these down here so that most important ones get processed last
					calls = atoi(TheQueues.at(i).GetCalls().c_str());
					abandons = atoi(TheQueues.at(i).GetAbandons().c_str());
					linesdialing = TheCallCache->LinesDialing(queue);
					availagents = TheQueues.at(i).GetAvailAgents(TheAgents);
					linestodial = evaluate(mode, linesdialing, availagents, maxratio, maxlines, maxabandons, calls, abandons);
					unsigned int remaininglines = maxlines - linestodial;

					transfer = "";

					if (linestodial >= 5000 || remaininglines >= 5000)
					{
						std::cout << queue << ": linestodial or remaininglines are greater than 5000, something is WRONG!" << std::endl;
						std::cout << queue << ": ldg: " << linesdialing << " aa: " << availagents << " mr: " << maxratio << " ml: " << maxlines << " ma: " << maxabandons << " mode: " << mode << " calls: " << calls << " abs: " << abandons << " l2d: " << linestodial << " rls: " << remaininglines << std::endl;
					}
#ifdef DEBUG
					std::cout << "[DEBUG]" << queue << ": ldg: " << linesdialing << " aa: " << availagents << " mr: " << maxratio << " ml: " << maxlines << " ma: " << maxabandons << " mode: " << mode << " calls: " << calls << " abs: " << abandons << " l2d: " << linestodial << " rls: " << remaininglines << std::endl;
#endif
					// std::cout << queue << ": processing this campaign" << std::endl;

					// main dial loop for each campaign
					if (linestodial && mode != "closer" && mode != "inbound" && true)
					{
						if (debug)
						{
							if (calls != 0)
							{
								if (doColorize)
								{
									std::cout << std::setprecision(4) << queue << fg_light_cyan << ": ABANDON (percentage): " << static_cast<double>(abandons) / static_cast<double>(calls) * 100.0 << normal << std::endl;
								}
								else
								{
									std::cout << std::setprecision(4) << queue << ": ABANDON (percentage): " << static_cast<double>(abandons) / static_cast<double>(calls) * 100.0 << std::endl;
								}
							}
							else
							{ // No div by 0 allowed!
								if (doColorize)
								{
									std::cout << fg_light_cyan << "ABANDON (percentage): 0" << normal << std::endl;
								}
								else
								{
									std::cout << queue << ": ABANDON (percentage): 0" << std::endl;
								}
							}
						}

						// this is just a base to get the building of the query string going
						query = "SELECT id, phone FROM campaign_" + queue + " WHERE 1 ";

						if (calltoday != "true")
						{
							query += " AND (LEFT(lastupdated,10) = LEFT(NOW(),10) AND disposition = 1) OR LEFT(lastupdated,10) <> LEFT(NOW(),10) ";
						}

						if (filter.empty() == false && filter != "0" && filter != "None" && filter != "none")
						{
							if (debug)
							{
								std::cout << queue << ": filter - " << filter << std::endl;
							}
							query += " AND " + filter;
						}

						for (int x = 0; x < TheQueues.at(i).OccurencesOf("filters"); x++)
						{
							std::string fnum, fstring, enabled;
							fnum = TheQueues.at(i).GetSetting(x, "filters").GetAttribute("number");
							fstring = TheQueues.at(i).GetSetting(x, "filters").GetAttribute("string");
							enabled = TheQueues.at(i).GetSetting(x, "filters").GetAttribute("enable");
							if (enabled == "true")
							{
								if (debug)
								{
									std::cout << queue << ": filter - " << fstring << std::endl;
								}
								query += " AND " + fstring;
							}
						}

						if (f_areacode.empty() == false && f_areacode != "0")
						{
							if (debug)
							{
								std::cout << queue << ": f_areacode - " << f_areacode << std::endl;
							}
							query += " AND LEFT(phone,3)='" + f_areacode + "'";
						}

						if (f_areacode_prefix.empty() == false && f_areacode_prefix != "0")
						{
							if (debug)
							{
								std::cout << queue << ": f_areacode_prefix - " << f_areacode_prefix << std::endl;
							}
							query += " AND LEFT(phone,6)='" + f_areacode_prefix + "'";
						}

						if (f_zipcode.empty() == false && f_zipcode != "0")
						{
							if (debug)
							{
								std::cout << queue << ": f_zipcode - " << f_zipcode << std::endl;
							}
							query += " AND LEFT(zip,5)='" + f_zipcode + "'";
						}

						if (usetzfilter == "true")
						{
							tzearliest = TheQueues.at(i).GetSetting("tzearliest").Get();
							tzlatest = TheQueues.at(i).GetSetting("tzlatest").Get();

							query += " AND " + getFilter(tzearliest, tzlatest, isAHoliday);

							if (debug)
							{
								if (doColorize)
								{
									std::cout << queue << fg_green << ": tzFilter Enabled " << normal << std::endl;
								}
								else
								{
									std::cout << queue << ": tzFilter Enabled " << std::endl;
								}
							}
						}
						else
						{
							if (debug)
							{
								if (doColorize)
								{
									std::cout << queue << fg_red << ": tzFilter Disabled " << normal << std::endl;
								}
								else
								{
									std::cout << queue << ": tzFilter Disabled " << std::endl;
								}
							}
						}

						if (usednc == "true")
						{
							// query += " AND phone NOT IN (SELECT phone FROM DNC) ";
						}

						// query += " ORDER BY attempts + pickups ASC LIMIT " + itos(skip) + "," + itos(linestodial);
						query += " AND NOT EXISTS (SELECT 1 FROM DNC WHERE DNC.phone = campaign_" + queue + ".phone)";

						if (orderby == "id" || orderby == "phone")
						{
							if (orderby == "id")
							{
								query += " ORDER BY id ASC ";
							}
							if (orderby == "phone")
							{
								query += " ORDER BY phone ASC ";
							}
						}
						else
						{
							query += " ORDER BY attempts + pickups ASC ";
						}

						query += " LIMIT " + itos(skip) + "," + itos(linestodial);

						if (debug)
						{
							std::cout << queue << ": query - " << query << std::endl;
						}

						if (debug)
						{
							std::cout << queue << ": Dialing " << linestodial << " calls (" << skip << ") skipped" << std::endl;
						}
						if (mysql_query(mysql, query.c_str()) != 0)
						{
							std::cerr << "Error selecting leads from mysql! Did you run --tzpopulate?" << std::endl;
							// return 1;
							usleep(10000);
						}
						else
						{

							result = mysql_use_result(mysql);
							query = "UPDATE campaign_" + queue + " SET attempts=attempts+1 WHERE ";
							for (counter = 0; (row = mysql_fetch_row(result)); counter++)
							{
								if (counter)
								{
									query += " OR ";
								}
								query += " id=" + std::string(row[0]);
								TheCallCache->AddCall(row[1], queue, row[0], callerid, usecloser, dspmode, trunk, dialprefix, transfer, timeout);
							}

							/*
							 working area for failover primary queries
							 or some form or priorities

							 this one is a basic reset to first filter
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

							 this grabs all enabled filters
																for (int x = 0; x < TheQueues.at(i).OccurencesOf("filters"); x++) {
																		std::string fnum,fstring,enabled;
																		fnum = TheQueues.at(i).GetSetting(x,"filters").GetAttribute("number");
																		fstring = TheQueues.at(i).GetSetting(x,"filters").GetAttribute("string");
																		enabled = TheQueues.at(i).GetSetting(x,"filters").GetAttribute("enable");
																		if (enabled == "true") {
																				if (debug) {
																						std::cout << queue << ": filter - " << fstring << std::endl;
																				}
																				query += " AND " + fstring;
																		}
																}
							*/

							if (mysql_errno(mysql))
							{
								std::cerr << "Error fetching rows from mysql!" << std::endl;
								return 1;
							}
							if (!counter)
							{
								if (doColorize)
								{
									std::cout << queue << fg_light_red << ": has ran out of leads! (CHECK YOUR FILTERS!!!)" << normal << std::endl;
								}
								else
								{
									std::cout << queue << ": has ran out of leads! (CHECK YOUR FILTERS!!!)" << std::endl;
								}
							}
							else if (counter < linestodial)
							{
								std::cerr << queue << " is running very low on leads!" << std::endl;
							}

							mysql_free_result(result);
							if (counter)
							{
								TheQueues.rWhere(queue).AddCallsDialed(counter);
								TheQueues.rWhere(queue).WriteCalls();
								if (mysql_query(mysql, query.c_str()) != 0)
								{
									std::cerr << "Error updating leads in mysql!" << std::endl;
									return 1;
								}
							}
						}
					}

					// std::cout << "remaininglines: " << remaininglines << std::endl;
					// std::cout << "   currentTime: " << currentTime << " timeSinceLastCloserCheck: " << timeSinceLastCloserCheck << std::endl;

					if ((remaininglines && usecloser == "true" && closercam != "none" && true) && (currentTime - timeSinceLastQueueUpdate == 5))
					{

						transfer = "TRANSFER";

						int availclosers = TheQueues.rWhere(closercam).GetAvailAgents(TheAgents);

						std::cout << "[DEBUG]" << currentTime << ":" << queue << ": availclosers: " << availclosers << " - remaininglines: " << remaininglines << std::endl;

						query = "SELECT count(*) FROM campaign_" + queue + " WHERE ";
						query += " disposition = 12 AND closerdispo = 0 ";

						if (mysql_query(mysql, query.c_str()) != 0)
						{
							std::cout << "Error selecting records !" << std::endl;
						}
						else
						{

							queryResult = mysql_use_result(mysql);
							queryRow = mysql_fetch_row(queryResult);
							if (queryRow && queryRow[0])
							{
								int ccb_counter = std::stoi(std::string(queryRow[0]));
								if (debug)
								{
									if (ccb_counter)
									{
										std::cout << queue << ": Total Closer Callbacks Remaining: " << ccb_counter << std::endl;
									}
								}
							}
							mysql_free_result(queryResult);
						}

						// std::cout << queue << ": Testing - got here " << std::endl;

						if (availclosers)
						{
							query = "SELECT id, phone FROM campaign_" + queue + " WHERE ";
							query += " disposition = 12 AND closerdispo = 0 AND ((lastupdated) < DATE_SUB(NOW(),INTERVAL 3 MINUTE)) ";
							query += " ORDER BY attempts + pickups ASC LIMIT 1";
							// query += " ORDER BY attempts + pickups ASC LIMIT " + itos(selectLessorOf(remaininglines,availclosers));

							if (mysql_query(mysql, query.c_str()) != 0)
							{
								std::cerr << "Error selecting leads from mysql! Did you run --tzpopulate?" << std::endl;
								// return 1;
								usleep(10000);
							}
							else
							{
								result = mysql_use_result(mysql);

								query = "UPDATE campaign_" + queue + " SET attempts=attempts+1 WHERE ";
								for (counter = 0; (row = mysql_fetch_row(result)); counter++)
								{
									if (counter)
									{
										query += " OR ";
									}
									query += " id=" + std::string(row[0]);

									TheCallCache->AddCall(row[1], queue, row[0], callerid, usecloser, dspmode, trunk, dialprefix, transfer, timeout);
									if (debug)
									{
										std::cout << queue << ": Dialing Closer Callback Number: " << row[1] << std::endl;
									}
								}

								if (mysql_errno(mysql))
								{
									std::cerr << "Error fetching rows from mysql!" << std::endl;
									return 1;
								}

								mysql_free_result(result);

								if (counter)
								{
									TheQueues.rWhere(queue).AddCallsDialed(counter);
									TheQueues.rWhere(queue).WriteCalls();

									if (mysql_query(mysql, query.c_str()) != 0)
									{
										std::cerr << "Error updating leads in mysql!" << std::endl;
										return 1;
									}
								}
							}
						}
					}

					if ((remaininglines && usecallback == "true" && true) && (currentTime - timeSinceLastQueueUpdate == 5))
					{
						// if (remaininglines && usecallback == "true" && true) {
						/*
											if ((i != 0 && i % 10 == 0 && currentTime - timeSinceLastCallbackCheck > 5) \
													|| (currentTime - timeSinceLastCallbackCheck > 20 && i != 0)) {
						*/
						if (debug)
						{
							std::cout << "remaininglines: " << remaininglines << std::endl;
						}

						query = "SELECT id, phone FROM campaign_" + queue + " WHERE ";

						if (filter.empty() == false && filter != "0")
						{
							if (debug)
							{
								std::cout << queue << ": filter - " << filter << std::endl;
							}
							query += filter + " AND ";
						}

						if (usetzfilter == "true")
						{
							tzearliest = TheQueues.at(i).GetSetting("tzearliest").Get();
							tzlatest = TheQueues.at(i).GetSetting("tzlatest").Get();

							query += getFilter(tzearliest, tzlatest, isAHoliday) + " AND ";
							if (debug)
							{
								std::cout << queue << ": tzFilter Enabled " << std::endl;
							}
						}
						else
						{
							if (debug)
							{
								std::cout << queue << ": tzFilter Disabled" << std::endl;
							}
						}

						query += " disposition = 0 AND";
						query += " (cb_datetime IS NULL OR (cb_datetime > '2000-01-01 00:00:00' AND cb_datetime < NOW()))";
						query += " AND ((lastupdated) < DATE_SUB(NOW(),INTERVAL 2 MINUTE)) ";

						query += " ORDER BY attempts + pickups ASC LIMIT " + itos(remaininglines);

						if (mysql_query(mysql, query.c_str()) != 0)
						{
							std::cerr << "Error selecting leads from mysql! Did you run --tzpopulate?" << std::endl;
							// return 1; // let it continue
							usleep(10000);
						}
						else
						{
							result = mysql_use_result(mysql);
							// currently we set cb_datetime to '', we should leave it and set it to ''
							// connected to agent
							query = "UPDATE campaign_" + queue + " SET attempts=attempts+1,cb_datetime=NULL WHERE ";
							for (counter = 0; (row = mysql_fetch_row(result)); counter++)
							{
								if (counter)
								{
									query += " OR ";
								}
								query += " id=" + std::string(row[0]);

								TheCallCache->AddCall(row[1], queue, row[0], callerid, usecloser, dspmode, trunk, dialprefix, transfer, timeout);
								std::cout << "Dialing Callback: " << row[1] << std::endl;
							}

							if (mysql_errno(mysql))
							{
								std::cerr << "Error fetching rows from mysql!" << std::endl;
								return 1;
							}

							if (!counter)
							{
								std::cerr << queue << " has no Callbacks to make" << std::endl;
							}

							mysql_free_result(result);

							if (counter)
							{
								TheQueues.rWhere(queue).AddCallsDialed(counter);
								TheQueues.rWhere(queue).WriteCalls();

								if (mysql_query(mysql, query.c_str()) != 0)
								{
									std::cerr << "Error updating leads in mysql!" << std::endl;
									return 1;
								}
							}

							timeSinceLastCallbackCheck = currentTime;
						}
						//}
					}
				}
			}

			try
			{
				TheCallCache->CallAll(mainHost);
			}

			catch (xOutOfHosts)
			{
				std::cerr << "Exception: Ran out of hosts!" << std::endl;
				return 1;
			}

			catch (xForkError)
			{
				std::cerr << "Exception: Unable to fork the parent process!" << std::endl;
				return 1;
			}
		}

		mysql_close(mysql);

	} // Not nested for convenience.

	catch (const std::exception &e)
	{
		std::cerr << "Caught Exception: " << e.what() << std::endl;
		return 1;
	}

	catch (xLoopEnd e)
	{
		std::cout << "Caught exception while trying to get debug & log settings!" << std::endl;
		std::cout << e.what();
		std::cout << std::endl
				  << std::endl;
		return 1;
	}

	catch (const std::runtime_error &e)
	{
		// Handle the runtime_error exception
		std::cerr << "Caught a runtime_error exception: " << e.what() << std::endl;
		return 1;
	}

	catch (...)
	{
		std::cerr << "Unhandled Exception: This is most likely a STL issue." << std::endl;
		return 1;
	}

	return 0;
}
