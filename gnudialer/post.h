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
#include <fstream>
#include <stdio.h>
#include <sys/time.h>
#include <string>

#ifndef POST
#define POST

char hexToDec(char theHexChar) {
	if(theHexChar >= 'A') return (theHexChar & 0xdf) - 'A' + 10;
	else return theHexChar - '0';
	}

std::string dehexify(const std::string & original) {

	std::string result;
	for (unsigned int i = 0; i < original.length(); i++) {

		if (original[i] == '%') {
			result += hexToDec(original[i+1]) * 16 + hexToDec(original[i+2]);
			i+=2;
		} else {
			result += original[i];
			}
		}

	return result;

	}

//    else if(itsRawData[i] == '%') {
//      result += hexToDec(itsRawData[i + 1]) * 16 + hexToDec(itsRawData[i + 2]);
//      i += 2;
//      } else
//        result += itsRawData[i];
//      }

std::string hexify (const std::string &s ) {
	std:: ostringstream out;
	out << std::hex;
	for (unsigned int i = 0; i < s.size(); ++i )
		out << unsigned (s[i]);

	return out.str();
}

class Post {

public:

Post() {}
~Post() {}
Post(std::string contentType) {

	queryString = getenv("QUERY_STRING");

	if (!queryString.length()) {
		std::getline(std::cin,queryString,'\0');
	}

	queryString = "&" + queryString + "&";
}

std::string GetField(const std::string & field, int occurence) {

	std::string temp, result;
	occurence++;
	int pos = 0;
	for (int i = 0; i < occurence; i++)
		pos = (queryString).find("&" + field + "=",pos+1);
	pos += field.length() + 2;
	int end = queryString.find("&",pos);
	temp = queryString.substr(pos,end-pos);
	for (unsigned int i = 0; i < temp.length(); i++) {
		if (temp[i] == '+') result += " ";
		else result += temp[i];
		}
	return dehexify(result);

	}

bool HasField(const std::string & field) {

	if (queryString.find("&" + field + "=",0) == std::string::npos) {

		return false;

	} else {

		return true;

		}

	}

int OccurencesOf(const std::string & field) {

	int i = 0;
	int pos = 0;

	if (queryString.find("&" + field + "=",0) != std::string::npos) {

		for (; queryString.find("&" + field + "=",pos+1) != std::string::npos; i++)
			pos = (queryString).find("&" + field + "=",pos+1);

		return i;

	} else {

		return 0;

		}

	}

std::string GetField(std::string field) {
  std::string result, raw = __GetField(field);
  for (unsigned int i = 0; i < raw.length(); i++) {
    if (raw[i] == '+')
      result += " ";
    else {
      result += raw[i];
      }
    }
  return dehexify(result);
  }

std::string __GetField(std::string field) {
  int fieldstart = queryString.find("&" + field + "=",0);
  int valuestart = queryString.find("=",fieldstart+1) + 1;
  int valueend = queryString.find("&",valuestart);
  if (queryString.find("&" + field + "=",0) == std::string::npos) {
    return static_cast<std::string>("");
  } else {
    return queryString.substr(valuestart,valueend-valuestart);
    }

  }

void PrintHiddenQueryString() {
  std::cout << "<!-- ";
  std::cout << queryString;
  std::cout << "  -->";
  }

void LogHiddenQueryString() {
        if (queryString.find("mode=login",0) != std::string::npos) {
                //do nothing
        } else {
		writeLog("Dialeradmin: " + queryString);
	}
}


void __writeLog(const std::string & thelogstring, const std::string & filename) {

        timeval tv;
        gettimeofday(&tv,NULL);
        time_t thetime = tv.tv_sec;
        tm * ptm = localtime(&thetime);
        char mytimestamp[14];
        std::string theTimeStamp;

        sprintf(mytimestamp, "%4.4i%2.2i%2.2i%2.2i%2.2i%2.2i", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        theTimeStamp = std::string(mytimestamp);

        std::system(("touch " + filename).c_str());

        std::fstream OutFile;
        OutFile.open(filename.c_str(),std::ios::in|std::ios::out|std::ios::binary);
        OutFile.seekg(0,std::ios::end);
        OutFile << theTimeStamp << ": " << thelogstring << std::endl;
        OutFile.close();

}

void writeLog(const std::string & thelogstring) {

        __writeLog(thelogstring,"/tmp/dialeradmin.log");

}




private:

std::string queryString;

};

#endif
