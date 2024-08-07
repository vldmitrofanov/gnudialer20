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


#include "dispo.h"

int main(int argc, char** argv) {
std::string thedispo,campaign,agent,epoch,calltime,thedispofull,leadid;
for (std::string tempLine; std::getline(std::cin,tempLine,'\n');) {
        if (tempLine.empty()) {
                break;
                }
        }
        int d;
        for (int i = 1; i < argc; i++) {
                if (i == 1) {
                        thedispo = argv[i];
                        }
                if (i == 2) {
                        agent = argv[i];
                        }
                if (i == 3) {
                        campaign = argv[i];
                        }
                if (i == 4) {
                        epoch = argv[i];
                        }
                if (i == 5) {
                        calltime = argv[i];
                        }
                d = i;
                }

                if (d == 3) {
                        writeDispo(agent,campaign,thedispo);
                        writeDBDispo(agent,campaign,thedispo,leadid);
                        }
                if (d == 5) {
                        thedispofull = leadid + "," + epoch + "," + calltime + "," + thedispo;
                        writeDispoFull(agent,campaign,thedispofull);
                        }


	return 1;
	
}
