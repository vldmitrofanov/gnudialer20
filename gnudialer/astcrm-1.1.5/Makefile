#
# astcrm - General purpose CRM
#
# General purpose CRM software for asterisk
#
# Copyright (C) 2005, GnuDialer Project
#
# Heath Schultz <heath1444@yahoo.com>
#
# This program is free software, distributed under the terms of
#
# the GNU General Public License.
#

all:
	g++ -Wall -Wshadow crm.cpp -o crm.cgi
	g++ -Wall -Wshadow submitinfo.cpp -o submitinfo.cgi
	echo
	echo
	echo "If you get a compile error on AstMonitor.java, ignore it."
	echo
	echo
	javac AstMonitor.java

clean:
	rm *.cgi

install:
	chmod a+w /var/www/cgi-bin/
	cp crm.cgi /var/www/cgi-bin/
	cp submitinfo.cgi /var/www/cgi-bin/
	cp astcrm.conf /var/www/cgi-bin/
	chmod a+r /var/www/cgi-bin/astcrm.conf
	mkdir -p /var/www/html/crm
	cp *.html /var/www/html/crm/
	cp *.class /var/www/html/crm/
	cp *.gif /var/www/html/crm/

uninstall:
	rm -rf /var/www/html/crm/
	rm /var/www/cgi-bin/crm.cgi
	rm /var/www/cgi-bin/submitinfo.cgi

distclean:
	make clean
	rm Makefile
