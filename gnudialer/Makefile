#
# GnuDialer - Complete, free predictive dialer
#
# Complete, free predictive dialer for contact centers.
#
# Copyright (C) 2005, GnuDialer Project
#
# Heath Schultz <heath1444@yahoo.com>
# Richard Lyman <richard@dynx.net>
#
# This program is free software, distributed under the terms of
# the GNU General Public License.
#

SHELL=/bin/sh

# Feel free to comment out the "guessed values" and enter the paths explicitly
# if you have troubles getting them from ./mkhelper or mysql_config.

RUNAS := `ps wwwaux | grep asterisk  | sed -e 's/ /\n/' | sed -e 's/ /\n/' | head -n 1`
DIST := `./detect`

MYSQLVER := `mysql -V | cut -c25-27`    

CXXFLAGS = -std=c++11 -Wall -I/usr/include/mysql

# Linker flags
LDFLAGS = -L/usr/lib/mysql -lmysqlcppconn   

ifeq ($(shell `detect`),RedHat)
 CGIBIN := `./mkhelper CGIBIN`
 DOCROOT := `./mkhelper DOCROOT`
 #MYSQLLIBLOC := `mysql_config --libs | sed -e 's/ /\n/' | sed -e 's/ /\n/' | head -n 1`
 #MYSQLLIBLOC=
 LMYSQLCLIENT := `mysql_config --libs | sed -e 's/ /\n/' | sed -e 's/ /\n/' | head -n 2 | tail -n 1`
else
 CGIBIN=/var/www/cgi-bin/
 DOCROOT=/var/www/html/
 #MYSQLLIBLOC=/usr/lib/mysql
 #MYSQLLIBLOC=
 LMYSQLCLIENT=-lmysqlclient
endif

#sometimes you need to comment out all but UB=/usr/bin (if you are going to run as root
#ifeq (${RUNAS},root)
        UB=/usr/bin
#else
#        UB=/usr/bin/${RUNAS}
#endif

CC=g++ -Wall -Wshadow
#CC=g++ -Weffc++

all: printdistinfo printrunasinfo printcgibin printdocroot printasteriskv printmysqlver printmysqlinc thegnudialer dialeradmin agenthours stats newstats allstats up
	@echo
	@echo "Done! Now type \"make install\" to install."
	@echo

printdistinfo:

	@echo
	@echo "Checking Linux Dist Info..."
	@echo
	@echo "DISTRIBUTION: " ${DIST}
	@echo

printrunasinfo:

	@echo
	@echo "Asterisk being RUN..."
	@echo
	@echo "AS: " ${RUNAS}
	@echo
                                        
printcgibin:
	@echo
	@echo "WE GUESSED THAT THE PATH TO cgi-bin IS: " ${CGIBIN}
	@echo "I WILL TRY TO USE THIS PATH"
	@echo "IF THIS IS INCORRECT, PLEASE EDIT THE PATH MANUALLY IN Makefile"
	@echo
	
printdocroot:

	@echo
	@echo "Checking for Apache (httpd.conf)..."
	@echo
#@locate httpd.conf
	@echo
	@echo "OK."
	@echo
	@echo "WE GUESSED THAT THE PATH TO DocumentRoot IS: " ${DOCROOT}
	@echo "I WILL TRY TO USE THIS PATH"
	@echo "IF THIS IS INCORRECT, PLEASE EDIT THE PATH MANUALLY IN Makefile"
	@echo

printmysqlver:

	@echo
	@echo "Checking MySQL Version..."
	@echo
	@echo "MYSQL VERSION: " ${MYSQLVER}
	@echo
	@echo "THIS MUST SAY 4.1 OR HIGHER!!!!!!"
	@echo "CURRENTLY THERE ARE NO CHECKS TO STOP COMPILING IF TOO OLD!"                
	@echo

printmysqlinc:
	@echo
	@echo "Checking for mysql_config..."
	@echo
	@locate mysql_config
	@echo
	@echo "OK."
	@echo

printasteriskv:
	@echo
	@echo "Checking for asterisk..."
	@echo
	@/usr/sbin/asterisk -V
	@echo
	@echo "OK."
	@echo

thegnudialer:
	$(CC) gnudialer.cpp DBConnection.cpp -g -o gnudialer `mysql_config --include` `mysql_config --libs` $(CXXFLAGS) $(LDFLAGS)

#dialeradmin:
#	$(CC) ${MYSQLINC} ${MYSQLLIBLOC} ${LMYSQLCLIENT} dialeradmin.cpp -o dialeradmin.cgi

dialeradmin:
	$(CC) dialeradmin.cpp DBConnection.cpp -o dialeradmin.cgi `mysql_config --include` `mysql_config --libs` $(CXXFLAGS) $(LDFLAGS)

agenthours:
	$(CC) agenthours.cpp DBConnection.cpp -o agenthours.cgi $(CXXFLAGS) $(LDFLAGS)

stats:
	$(CC) stats.cpp DBConnection.cpp -o stats.cgi $(CXXFLAGS) $(LDFLAGS)

newstats:
	$(CC) newstats.cpp DBConnection.cpp -o newstats.cgi $(CXXFLAGS) $(LDFLAGS)

allstats:
	$(CC) allstats.cpp DBConnection.cpp -o allstats.cgi $(CXXFLAGS) $(LDFLAGS)
	        
up:
	$(CC) up.cpp DBConnection.cpp -o up.cgi $(CXXFLAGS) $(LDFLAGS)
  
clean:
	rm gnudialer dialeradmin.cgi agenthours.cgi stats.cgi up.cgi $(CXXFLAGS) $(LDFLAGS)

install: printdistinfo printrunasinfo printmysqlver printmysqlinc printcgibin printdocroot theinstall

theinstall:
	cp gnudialer ${UB}
	cp *.cgi ${CGIBIN}
	mkdir -p ${DOCROOT}gnudialer
	mkdir -p /usr/lib/gnudialer
	chmod a+rw /usr/lib/gnudialer
	mkdir -p /usr/lib/gnudialer/timezones
	chmod a+rw /usr/lib/gnudialer/timezones
	cp ./timezones/* /usr/lib/gnudialer/timezones/
	mkdir -p /var/log/asterisk/gnudialer
	chmod a+rw /var/log/asterisk/gnudialer
	chmod a+rw /etc/asterisk/queues.conf
	chmod a+rw /etc/asterisk/agents.conf
	chmod a+rw ${CGIBIN}
	cp *.html ${DOCROOT}gnudialer/
	cp *.gif ${DOCROOT}gnudialer/
	cp *.script ${CGIBIN}
	cp dialer.conf /etc/asterisk/dialer.conf
	mkdir -p /tmp/exports
	mkdir -p /tmp/sales
	chmod 755 /tmp/exports
	chmod 755 /tmp/sales
	touch /tmp/bogus.helper.1-1-1980         
	chmod 666 /tmp/*.helper.*
	@echo
	@echo "Done!"
	@echo

uninstall: printdocroot printcgibin
	rm ${UB}gnudialer
	rm ${CGIBIN}dialeradmin.cgi
	rm ${CGIBIN}stats.cgi
	rm ${CGIBIN}agenthours.cgi
	rm ${CGIBIN}*.script
	rm ${DOCROOT}gnudialer/dialerlogin.html
	rm ${DOCROOT}gnudialer/checkered.gif
	rm -rf /usr/lib/gnudialer/timezones/
	rm -rf /tmp/exports
	rm -rf /tmp/sales
         