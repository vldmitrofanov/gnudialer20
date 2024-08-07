/*
 * astcrm - General purpose CRM                       
 *
 * General purpose CRM software for asterisk
 * 
 * Copyright (C) 2005, GnuDialer Project
 *
 * Heath Schultz <heath1444@yahoo.com>
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License.
 */

#ifndef COMMAND
#define COMMAND

#include <iostream>
#include <string>
#include <stdio.h>

std::string command (const std::string& command) {
	std::string output;

	FILE* f = popen (command.c_str (), "r"); 
  
	if (f != 0) { 
		char line[256]; 
		char* ptr;
		while ((ptr = fgets (line, sizeof (line), f)) != NULL) output += ptr;
		pclose (f);
	}

	return output;

}

#endif
