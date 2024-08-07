#include <iostream>
#include <string>

std::string cam2closer(const std::string & orig) {

	std::ifstream QueuesIn;
	QueuesIn.open("/etc/asterisk/queues.conf");
	for (std::string tempLine; std::getline(QueuesIn,tempLine,'\n'); ) {

		if (tempLine.find(("[" + orig + "]"),0) != std::string::npos && tempLine.find(";",0) == std::string::npos) {

			while (std::getline(QueuesIn,tempLine,'\n')) {

				if (tempLine.find(";:setting:closercam:",0) != std::string::npos) {

					return tempLine.substr(20,tempLine.length()-20);

					}

				}

			}

		}
	QueuesIn.close();

	return static_cast<std::string>("none");

	}
