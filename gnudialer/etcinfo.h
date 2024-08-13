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

inline std::string getEtcLine(int line) {
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

inline std::string getGnuDialerPassword() { return getEtcLine(1); }
inline std::string getMySqlUser() { return getEtcLine(2); }
inline std::string getMySqlPass() { return getEtcLine(3); }
inline std::string getMySqlPassword() { return getMySqlPass(); }
inline std::string getMySqlHost() { return getEtcLine(4); }
inline std::string getMySqlHostname() { return getEtcLine(4); }
inline std::string getDbName() { return getEtcLine(5); }
inline std::string getMysqlPort() { return getEtcLine(6); }
inline std::string getManagerUser() { return getEtcLine(7); }
inline std::string getManagerUsername() { return getEtcLine(7); }
inline std::string getManagerPassword() { return getEtcLine(8); }
inline std::string getManagerPass() { return getEtcLine(8); }
inline std::string getMainHost() { return getEtcLine(9); }
inline std::string getServerId() { return getEtcLine(10); }

#endif
