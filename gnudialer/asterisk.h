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

#include <queue>
#include "exceptions.h"

#ifndef ASTERISK
#define ASTERISK

class Call;

class Asterisk
{

public:
	Asterisk(const std::string &amiLine, const std::string &ariLine)
	{

		std::string tWeight;
		std::stringstream HostStream;
		HostStream << amiLine;

		if (std::getline(HostStream, host, ',') &&
			std::getline(HostStream, port, ',') &&
			std::getline(HostStream, user, ',') &&
			std::getline(HostStream, pass, ',') &&
			std::getline(HostStream, tWeight, ','))
		{

			weight = atoi(tWeight.c_str());

			if (weight < 1)
			{

				throw xInvalidWeightValue();
			}

			if (std::getline(HostStream, tWeight, ','))
			{

				throw xTooManyFields();
			}
		}
		else
		{

			throw xTooFewFields();
		}
		std::stringstream AriStream(ariLine);

		if (std::getline(AriStream, ariHost, ',') &&
			std::getline(AriStream, ariPort, ',') &&
			std::getline(AriStream, ariUser, ',') &&
			std::getline(AriStream, ariPass, ','))
		{
			// Successfully parsed ARI credentials
		}
		else
		{
			throw xTooFewFields(); // or another appropriate exception
		}
	}

	~Asterisk() {}

	const std::string &GetHost() const { return host; }
	const std::string &GetPort() const { return port; }
	unsigned int GetPortInt() const { return atoi(port.c_str()); }
	const std::string &GetUser() const { return user; }
	const std::string &GetPass() const { return pass; }
	const std::string &GetAriHost() const { return ariHost; }
	const std::string &GetAriPort() const { return ariPort; }
	unsigned int GetAriPortInt() const { return atoi(ariPort.c_str()); }
	const std::string &GetAriUser() const { return ariUser; }
	const std::string &GetAriPass() const { return ariPass; }
	const unsigned int &GetWeight() const { return weight; }

private:
	std::string host, port, user, pass, ariHost, ariPort, ariUser, ariPass;
	unsigned int weight;
};

class AsteriskList
{

public:
	AsteriskList()
	{
		currentWeightVal = 0;
		std::ifstream GDHosts;
		GDHosts.open("/etc/gdhosts.conf");
		if (!GDHosts)
		{
			throw xFileOpenError("gdhosts.conf");
		}
		else
		{
			std::string amiLine;
			while (std::getline(GDHosts, amiLine, '\n'))
			{

				// Check that the line is not a comment or empty
				if (amiLine.length() > 5 && amiLine.find("#", 0) == std::string::npos)
				{
					std::string ariLine;

					// Read the second line for ARI parameters
					if (std::getline(GDHosts, ariLine, '\n'))
					{
						// Push the parsed data into the list
						itsAsterisks.push_back(Asterisk(amiLine, ariLine));
					}
					else
					{
						std::cerr << "Expected ARI parameters on the second line" << std::endl;
						throw std::runtime_error("Expected ARI parameters on the second line");
					}
				}
			}

			if (itsAsterisks.empty())
			{
				throw xNoHostsDefined();
			}
		}
		GDHosts.close();
	}

	~AsteriskList() {}

	void pop_front()
	{

		itsAsterisks.pop_front();
	}

	const unsigned int size() const { return itsAsterisks.size(); }

	const Asterisk &Rotate()
	{

		if (itsAsterisks.size() == 1)
		{

			return itsAsterisks.front();
		}
		else
		{

			if (currentWeightVal + 1 < itsAsterisks.front().GetWeight())
			{

				currentWeightVal++;
				return itsAsterisks.front();
			}
			else
			{

				currentWeightVal = 0;
				itsAsterisks.push_back(itsAsterisks.front());
				itsAsterisks.pop_front();
				return itsAsterisks.front();
			}
		}
	}

private:
	void push_back(const std::string &amiLine, const std::string &ariLine)
	{
		Asterisk TheAsterisk(amiLine, ariLine);
		itsAsterisks.push_back(TheAsterisk);
	}

	std::deque<Asterisk> itsAsterisks;

	unsigned int currentWeightVal;
};

#endif
