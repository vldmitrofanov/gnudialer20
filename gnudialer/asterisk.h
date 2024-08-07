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

class Asterisk {

public:

Asterisk(const std::string & rawLine) {

	std::string tWeight;
	std::stringstream HostStream;
	HostStream << rawLine;

	if (std::getline(HostStream,host,',') && \
	    std::getline(HostStream,port,',') && \
	    std::getline(HostStream,user,',') && \
	    std::getline(HostStream,pass,',') && \
	    std::getline(HostStream,tWeight,',')) {

		weight = atoi(tWeight.c_str());

		if (weight < 1) {

			throw xInvalidWeightValue();

			}

		if (std::getline(HostStream,tWeight,',')) {

			throw xTooManyFields();

			}

	} else {

		throw xTooFewFields();

		}

	}

~Asterisk() {}

const std::string & GetHost()const { return host; }
const std::string & GetPort()const { return port; }
unsigned int GetPortInt()const { return atoi(port.c_str()); }
const std::string & GetUser()const { return user; }
const std::string & GetPass()const { return pass; }
const unsigned int & GetWeight()const { return weight; }

private:

std::string host, port, user, pass;
unsigned int weight;

};

class AsteriskList {

public:

AsteriskList() {

	currentWeightVal = 0;
	std::ifstream GDHosts;
	GDHosts.open("/etc/gdhosts.conf");
	if (!GDHosts) {
		throw xFileOpenError("gdhosts.conf");
	} else {
		for (std::string tempLine; std::getline(GDHosts,tempLine,'\n'); ) {

			if (tempLine.length() > 5 && \
			    tempLine.find("#",0) == std::string::npos) {
				push_back(tempLine);
				}

			}
		if (!itsAsterisks.size()) {
			throw xNoHostsDefined();
			}
		}

	GDHosts.close();

	}

~AsteriskList() {}

void pop_front() {

	itsAsterisks.pop_front();

	}

const unsigned int size()const { return itsAsterisks.size(); }

const Asterisk & Rotate() {

	if (itsAsterisks.size() == 1) {

		return itsAsterisks.front();

	} else {

		if (currentWeightVal + 1 < itsAsterisks.front().GetWeight()) {

			currentWeightVal++;
			return itsAsterisks.front();

		} else {

			currentWeightVal = 0;
			itsAsterisks.push_back(itsAsterisks.front());
			itsAsterisks.pop_front();
			return itsAsterisks.front();

			}

		}

	}

private:

void push_back(const std::string & tempLine) {
	Asterisk TheAsterisk(tempLine);
	itsAsterisks.push_back(TheAsterisk);
	}

std::deque<Asterisk> itsAsterisks;

unsigned int currentWeightVal;

};

#endif
