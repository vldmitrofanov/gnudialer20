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

#include <iostream>
#include <fstream>
#include <string>

#ifndef ETCINFO
#define ETCINFO

std::string getEtcLine(int line) {
	std::string etcLine;
	std::ifstream EtcInfoIn;
	EtcInfoIn.open("/etc/gnudialer.conf");
	if (EtcInfoIn)
		for (int i = 0; i < line; i++)
			std::getline(EtcInfoIn,etcLine,'\n');
		else std::cerr << "Error opening /etc/gnudialer.conf!" << std::endl;
	EtcInfoIn.close();
	return etcLine;
}

std::string getGnuDialerPassword() { return getEtcLine(1); }
std::string getMySqlUser() { return getEtcLine(2); }
std::string getMySqlPass() { return getEtcLine(3); }
std::string getMySqlPassword() { return getMySqlPass(); }
std::string getMySqlHost() { return getEtcLine(4); }
std::string getMySqlHostname() { return getEtcLine(4); }
std::string getDbName() { return getEtcLine(5); }
std::string getManagerUser() { return getEtcLine(6); }
std::string getManagerUsername() { return getEtcLine(6); }
std::string getManagerPassword() { return getEtcLine(7); }
std::string getManagerPass() { return getEtcLine(7); }
std::string getMainHost() { return getEtcLine(8); }

#endif
