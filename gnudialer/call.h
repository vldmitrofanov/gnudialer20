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

#include <queue>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <vector>
#include <curl/curl.h>
#include "asterisk.h"
#include "exceptions.h"
#include "Socket.h"
#include "itos.h"
#include "agent.h"

#ifndef CALL
#define CALL

const bool doColor = true;
const std::string neon = "\033[1;32m"; // set foreground color to light green
const std::string norm = "\033[0m";	   // reset to system

class Asterisk;

class Call;
std::string workingCampaign;
bool countCallsForCampaign(const Call &);

class Call
{

public:
	Call(const std::string &number,
		 const std::string &campaign,
		 const std::string &leadid,
		 const std::string &callerid,
		 const std::string &usecloser,
		 const std::string &dspmode,
		 const std::string &trunk,
		 const std::string &dialprefix,
		 const std::string &transfer,
		 const unsigned short int &timeout)
	{

		itsNumber = number;
		itsCampaign = campaign;
		itsLeadId = leadid;
		itsCallerId = callerid;
		itsUseCloser = usecloser;
		itsDSPMode = dspmode;
		itsTrunk = trunk;
		itsDialPrefix = dialprefix;
		itsTransfer = transfer;
		itsTimeout = timeout;
		called = false;
		answered = false;

		timeval tv;
		gettimeofday(&tv, NULL);
		itsTime = tv.tv_sec % 1000000;
	}

	const std::string &GetNumber() const { return itsNumber; }
	const std::string &GetCampaign() const { return itsCampaign; }
	const std::string &GetLeadId() const { return itsLeadId; }
	const std::string &GetCallerId() const { return itsCallerId; }
	const std::string &GetTrunk() const { return itsTrunk; }
	const std::string &GetDialPrefix() const { return itsDialPrefix; }
	const std::string &GetTransfer() const { return itsTransfer; }
	const unsigned long int &GetTime() const { return itsTime; }
	const unsigned short int &GetTimeout() const { return itsTimeout; }

	const bool &HasBeenCalled() const { return called; }
	const bool &HasBeenAnswered() const { return answered; }

	void SetAnswered() { answered = true; }

	void DoCall(const Asterisk &TheAsterisk, const std::string &mainHost)
	{

		std::string response;

		called = true;

		signal(SIGCLD, SIG_IGN);

		int pid = fork();

		if (pid == 0)
		{

			usleep(rand() % 2000000);

			CURL *curl;
			CURLcode res;

			/*
				ClientSocket AsteriskManager(TheAsterisk.GetHost(),TheAsterisk.GetPortInt());
				AsteriskManager >> response;
				AsteriskManager << "Action: Login\r\nUserName: " + \
						TheAsterisk.GetUser() + \
						"\r\nSecret: " + \
						TheAsterisk.GetPass() + \
						"\r\nEvents: off\r\n\r\n";
				AsteriskManager >> response;

				AsteriskManager << "Action: Originate\r\n";
		//		AsteriskManager << "Channel: iax2/gnudialer@" + TheAsterisk.GetHost() + "/" + itsNumber + "\r\n";

					if (itsTrunk.find("%3A",0) != std::string::npos) {
							int trunkPos = itsTrunk.find("%3A",0);
							itsTrunk.erase(trunkPos,3);
							itsTrunk.insert(trunkPos,":");
					}

						if (itsTrunk != "none") {
					if (itsDialPrefix == "none") {
								AsteriskManager << "Channel: " + itsTrunk + "/" + itsNumber + "\r\n";
		//              	          AsteriskManager << "Channel: Local/" + itsNumber + "@gdincoming/n\r\n";
					} else {
									AsteriskManager << "Channel: " + itsTrunk + "/" + itsDialPrefix + itsNumber + "\r\n";
		//                		AsteriskManager << "Channel: Local/" + itsDialPrefix + itsNumber + "@gdincoming/n\r\n";
					}
				} else {
					std::cout << mainHost << ": NO TRUNK DEFINED! (currently set to: " + itsTrunk + ")" << std::endl;
					exit(0);
				}
				if (itsTransfer == "TRANSFER") {
								AsteriskManager << "Context: gdtransfer\r\n";
				} else {
						AsteriskManager << "Context: gdincoming\r\n";
				}
				AsteriskManager << "Extension: s\r\n";
				AsteriskManager << "Priority: 1\r\n";
				AsteriskManager << "Variable: __LEADID=" + itsLeadId + "\r\n";
						AsteriskManager << "Variable: __CAMPAIGN=" + itsCampaign + "\r\n";
				AsteriskManager << "Variable: __DSPMODE=" + itsDSPMode + "\r\n";
						AsteriskManager << "Variable: __ISTRANSFER=" + itsTransfer + "\r\n";
						AsteriskManager << "Account: " + itsCampaign + "\r\n";
				AsteriskManager << "Async: true\r\n";
				AsteriskManager << "Timeout: " + itos(itsTimeout) + "\r\n";

				if (itsCallerId == "0123456789") {
					itsCallerId = itsNumber;
				}
						if (itsCallerId == "0123459999") {
					itsCallerId = itsNumber.substr(1,6);
				}

				AsteriskManager << "CallerID: ~" + itsCampaign + "-" + itsLeadId + "-" + itsUseCloser + "~ <" + itsCallerId + ">\r\n\r\n";

		// buffer fix

				AsteriskManager >> response;
				AsteriskManager << "Action: Logoff\r\n\r\n";
				AsteriskManager >> response;

		// end buffer fix

		//		std::cout << TheAsterisk.GetHost() << std::endl;
		//		std::cout << TheAsterisk.GetPort() << std::endl;
		//		std::cout << TheAsterisk.GetUser() << std::endl;
		//		std::cout << TheAsterisk.GetPass() << std::endl;

						if (doColor) {
					std::cout << mainHost << neon << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << norm << std::endl;
						} else {
					std::cout << mainHost << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << std::endl;
						}

				usleep(10000000);

				exit(0);
		*/

			curl = curl_easy_init();
			// std::cout << "ARI creds: " <<  TheAsterisk.GetAriHost() << ":" + TheAsterisk.GetAriPort() + " - " + TheAsterisk.GetAriUser() + ":" + TheAsterisk.GetAriPass() << std::endl;
			if (curl)
			{
				std::string url = "http://" + TheAsterisk.GetAriHost() + ":" + TheAsterisk.GetAriPort() + "/ari/channels";
				std::string dialPrefix = (itsDialPrefix == "none") ? "" : itsDialPrefix;
				std::string finalNumber = dialPrefix + itsNumber;
				std::cout << "TRUNK: " + itsTrunk <<std::endl;
				size_t pos = itsTrunk.find('!');
				while (pos != std::string::npos) {
					itsTrunk.replace(pos, 1, ":");
					pos = itsTrunk.find('!', pos + 1);
				}
				pos = itsTrunk.find("_EXTEN_");
				if (pos != std::string::npos) {
					// Replace _EXTEN_ with the actual number
					itsTrunk.replace(pos, 7, finalNumber);  // 7 is the length of "_EXTEN_"
				} else {
					throw std::runtime_error("Placeholder _EXTEN_ not found in the trunk string. Trunk example: SIP/faketrunk/sip=_EXTEN_@127.0.0.1=5062");
				}
				std::string postFields = "endpoint=" + itsTrunk +
										 "&extension=" + dialPrefix + itsNumber +
										 "&context=" + (itsTransfer == "TRANSFER" ? "gdtransfer" : "gdincoming") +
										 "&priority=1" +
										 "&callerId=" + itsCampaign + "-" + itsLeadId + "-" + itsUseCloser +
										 "&timeout=" + itos(itsTimeout) +
										 "&variables[__LEADID]=" + itsLeadId +
										 "&variables[__CAMPAIGN]=" + itsCampaign +
										 "&variables[__DSPMODE]=" + itsDSPMode +
										 "&variables[__ISTRANSFER]=" + itsTransfer +
										 "&account=" + itsCampaign;
				curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
				curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
				curl_easy_setopt(curl, CURLOPT_USERNAME, TheAsterisk.GetAriUser().c_str());
				curl_easy_setopt(curl, CURLOPT_PASSWORD, TheAsterisk.GetAriPass().c_str());

				// Perform the request, res will get the return code
				res = curl_easy_perform(curl);
				if (res != CURLE_OK)
				{
					std::string errorMessage = "curl_easy_perform() failed: " + std::string(curl_easy_strerror(res));
					std::cerr << errorMessage << std::endl;
					curl_easy_cleanup(curl);
					throw std::runtime_error(errorMessage);
				}
				else
				{
					std::cout << mainHost << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << std::endl;
				}

				// Cleanup
				curl_easy_cleanup(curl);
			}
			if (doColor)
			{
				std::cout << mainHost << neon << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << norm << std::endl;
			}
			else
			{
				std::cout << mainHost << ": " + itsCampaign + " - " + itsNumber + " - " + itsLeadId + " - " + itsUseCloser << std::endl;
			}

			usleep(10000000);

			exit(0);
		}

		if (pid == -1)
		{

			throw xForkError();
		}
	}

	~Call() {}

private:
	std::string itsNumber, itsCampaign, itsLeadId, itsCallerId, itsUseCloser, itsDSPMode, itsTrunk, itsDialPrefix, itsTransfer;
	unsigned long int itsTime;
	unsigned short int itsTimeout;
	bool called, answered;
};

class CallCache
{

public:
	CallCache() {}
	~CallCache() {}

	void push_back(const Call &TheCall)
	{

		itsCalls.push_back(TheCall);
	}

	void AddCall(const std::string &phone,
				 const std::string &campaign,
				 const std::string &leadid,
				 const std::string &callerid,
				 const std::string &usecloser,
				 const std::string &dspmode,
				 const std::string &trunk,
				 const std::string &dialprefix,
				 const std::string &transfer,
				 const unsigned short int &itsTimeout)
	{

		Call TheCall(phone, campaign, leadid, callerid, usecloser, dspmode, trunk, dialprefix, transfer, itsTimeout);
		itsCalls.push_back(TheCall);
	}

	void SetAnswered(const std::string &campaign, const std::string &leadid)
	{

		for (unsigned int i = 0; i < itsCalls.size(); i++)
		{

			if (itsCalls.at(i).GetCampaign() == campaign && itsCalls.at(i).GetLeadId() == leadid)
			{
				itsCalls.at(i).SetAnswered();
			}
		}
	}

	unsigned int LinesDialing(const std::string &campaign)
	{

		workingCampaign = campaign;

		timeval tv;
		gettimeofday(&tv, NULL);

		for (unsigned long int cur = tv.tv_sec % 1000000; itsCalls.size() && cur -
																					 itsCalls.front().GetTime() >
																				 itsCalls.front().GetTimeout() / 1000;)
		{

			itsCalls.pop_front();
		}

		return std::count_if(itsCalls.begin(), itsCalls.end(), countCallsForCampaign);
	}

	void CallAll(const std::string &mainHost)
	{

		for (unsigned int i = 0; i < itsCalls.size(); i++)
		{
			if (!itsCalls.at(i).HasBeenCalled())
			{
				try
				{
					itsCalls.at(i).DoCall(itsAsteriskList.Rotate(), mainHost);
				}
				catch (const xConnectionError &e)
				{
					std::cerr << "Exception: Unable to connect to " << e.GetHost() << "! Disabling host." << std::endl;
					itsAsteriskList.pop_front();
					if (!itsAsteriskList.size())
					{
						std::cout << "Out of hosts!" << std::endl;
						throw xOutOfHosts();
					}
				}
			}
		}
	}

private:
	std::deque<Call> itsCalls;
	AsteriskList itsAsteriskList;
};

bool countCallsForCampaign(const Call &TheCall)
{

	if (TheCall.GetCampaign() == workingCampaign && TheCall.HasBeenAnswered() == false)
	{

		return true;
	}
	else
	{

		return false;
	}
}

#endif
