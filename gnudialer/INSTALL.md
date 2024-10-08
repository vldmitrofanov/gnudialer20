Please follow these instructions carefully...

If you don't follow every single instruction
correctly, it most certainly will not work.


Suggestion:  	try using Sarge or FC3 (those are what the developers use)
		certain dists (like FC5), are known to have issues.


ASTERISK INSTALLATION INSTRUCTIONS
----------------------------------

	1. make && make install libpri

	2. make && make install zaptel (make sure you have
	   some type of timing device. ie. ztdummy or any ZAP device)
	   If you're using ztdummy, edit /etc/zaptel.conf and add
		
	   loadzone=us
	   defaultzone=us	

	3. Re-install libpri

	4. Included in this package is manager.c.originate.diff.txt
		Depending on what version of asterisk you have this may already
		be in your asterisk.  If not then, before making asterisk, apply it 
		to manager.c in your asterisk working dir

	5. Included in this package is manager.c.sendevent.diff.txt
		Asterisk has not included this patch yet, so it is required.  
		Before making asterisk apply it to manager.c in your asterisk working dir. 

	6. We currently do NOT require this (but might in the future)
		It may be a good idea to apply Matt Florell's patch to asterisk
	   	You can get it at http://bugs.digium.com/view.php?id=4297 
	   	Download the patch and mv res_features_CVS.patch5 
		(or whatever it is) to /usr/src/asterisk/res/
	   	Then patch patch -p1 < res_features_CVS.patch5
	   	(res_features.c.bridge.diff.txt added, twisteds' version of the above patch)

	7. make && make install 	your Asterisk 

	8. make samples			this sets up your config files and makes it so it is 
					ready for the mods we make to agents.conf and queues.conf
	   
	   ** OR ** make sure these these exist

	   /etc/asterisk/queues.conf contains

	   ** these sections change so make sure you read 
	   settings.h for what settings are currently required.
 
	   [general]
	   ;:setting:debug:true
	   ;:setting:log:true

	   [yourcampaignname]
	   ;:setting:active:true
	   ;:setting:function:predictive
	   ;:setting:maxratio:2.0
	   ;:setting:maxabandons:3.0
	   ;:setting:maxlines:23
	   ;:setting:sleeptime:1000
	   ;:setting:tzearliest:9
	   ;:setting:tzlatest:21
	   ;:setting:timeout:20000
	   ;:setting:dspmode:none
	   ;:setting:trunk:Zap/g1
	   ;:setting:dialprefix:none
	   ;:setting:usecloser:true
	   ;:setting:callerid:9999999999
	   ;:setting:filter:0
	   ;:setting:calls:0
	   ;:setting:abandons:0
	   ;:setting:totalcalls:0
	   ;:setting:totalabandons:0
	   ;:setting:closercam:CLOSER
	   ;:setting:talktime:0
	   ;:setting:totaltalktime:0
	   ;:setting:usecallback:false
	   ;:setting:usetzfilter:false
	   ;:setting:debug:true
	   ;:setting:skip:0
	   ;:setting:f_mode:0
	   ;:setting:f_areacode:0
	   ;:setting:f_areacode_prefix:0
	   ;:setting:f_zipcode:0
	   ;:setting:filters:number:0:enable:true:string:((disposition > -6 AND disposition < 6) AND attempts < 4)
	   ;:setting:filters:number:1:enable:false:string:((disposition > -6 AND disposition < 6) AND (attempts > 3 AND attempts < 8))
	   ;:setting:filters:number:2:enable:false:string:((disposition > -6 AND disposition < 6) AND attempts > 7)
	   ;:setting:filters:number:3:enable:false:string:(disposition = -3 OR disposition = 3)
	   ;:setting:filters:number:4:enable:false:string:(disposition = 0)
	   ;:setting:filters:number:5:enable:false:string:(disposition = -7)

5. Install astcrm

	astcrm is included in your gnudialer package. Just make and make install 

	(BE SURE TO CHECK THAT THE MAKEFILE IS SETTING PROPER FILE LOCATIONS)  
	
	The Makefile is mostly tested on Fedora Core 3 and Debian.  
	It may require some hacking for it to work on your setup.  
	Included is a Makefile used on Fedora Core 3.  
	It may or may not compile on yours. 
	(open it up and see what parameters might be different on your system)


OTHER REQUIRED PROGRAMS
-----------------------

	1. Apache - Any newer version will do.  (Tested with 1.3 and 2.0)

	2. MySQL - A newer version of the client and server programs is required. (4.1+)
		This is because of some the features we use do not exist in older ones.
		
	3. mysqlclient and mysqlclient-dev (aka. MySQL-client and MySQL-devel package)

	4. Asterisk PBX Head preferred (tested on 07/29/05 to 11/01/05)
	   
		http://www.dynx.net/ASTERISK/gnudialer/all-asterisk-20051201.tar.gz
		This already has the patches applied, and is known to work with gnudialer.


GETTING THE DIALPLAN TO WORK WITH GNUDIALER
----------------------------------------------

	1. Add "#include dialer.conf" in extensions.conf

	2. Add the following to your GLOBALS:
		** these were removed **
	
	3.	Look through dialer.conf.  
		You will need to understand it before you try and use gnudialer.
		It may also be wise for you to thumb through the code as well.  
		Also post your experience on the forum regarding the install.  
		That may aid others!

	4. Create the following entry in manager.conf:

		[dialer]
		secret = 1234
		read = system,call,log,verbose,command,agent,user
		write = system,call,log,verbose,command,agent,user
		permit=0.0.0.0/0.0.0.0

		[crm]
		secret = 1234
		read = call,agent,user 		; You can remove call but */#XX on the phone but doing so won't flush the screen
		write = call,agent,user 	; So remove it if you want some wrap up time between calls
		permit=0.0.0.0/0.0.0.0 

	5. Add to general something as follows:
		[general]
		enabled = yes 		# required!
		port = 5038  		# If you change this make sure gnduialer knows
		bindaddr = 0.0.0.0  	# some people have issues when they bind to a secondary nic

	6. Setup your Least Cost Routing extension in dialer.conf

		It must be named:					-|
									 | 
		[gnudialer]						 | this section not needed anymore	
		exten => _NXXNXXXXXX,1,Dial(sip/context/${EXTEN})	 | we added per campaign trunk setting 
		exten => _NXXNXXXXXX,2,Hangup				-|

		Gnudialer Puff Series allows you to cluster certain aspects of the dialer.
		On your main gnudialer server add [backtomain] if you use an originate server.
		exten => s,1,GoTo(gdincoming,s,1)
	
		So your server assigned in /etc/gdhosts.conf would
		be assigned a context that would call your main gnudialer server.
		You can use any method you prefer. 
		We mostly used IAX2 trunks between asterisk boxes for testing and 
		Zap for production boxes.

		An example of our sample context:
		(this is old, and here for reference only)

			[backtomain]
			exten => s,1,SetVar(LENGTH=${LEN(CALLERIDNAME)})
			exten => s,2,SetVar($$LENGTH-2)
			exten => s,3,SetVar(CALLFILENAME=${CALLERIDNAME:1:${LENGTH}}-${TIMESTAMP})
			exten => s,4,Monitor(gsm,${CALLFILENAME},m)
			exten => s,5,Dial(iax2/10@03/s)
			exten => s,6,Hangup
			exten => t,1,Hangup


	9. Edit astcrm.conf in your cgi-bin.
		
		* leave-this
		* SQL User
		* SQL Pass
		* SQL Host
		* SQL DB


	8. If you are using NVFaxDetect (might not work anymore), 
		you need to use one of the following codecs: ulaw, alaw, slin.  
		The same holds true with other methods of answering machine detection.
	   	ie, WaitForSilence and BackgroundDetect.

	

INSTALLING GNUDIALER
--------------------

	1. Edit gnudialer.conf in the working dir to match your /etc/gnudialer.conf 
		(that way when you update via make install it won't overwrite your settings)

	
			Here's how it should be...

		* First line is your 	GnuDialer password
		* Second line is your 	MySQL username
		* Third line is your 	MySQL password
		* Fourth line is your 	MySQL hostname
		* Fifth line is the 	db name the dialer should use
		* Sixth line is your 	manager username
		* Seventh line is your 	manager secret	
		* Eight line is your 	main host (normally localhost)

	2. make && make install		for gnudialer

	3. Create a database for the dialer in MySQL.

		Use the same name that you specified in gnudialer.conf. :)
		We suggest 'dialer'

	4. Make mysql-server run at boot with your preferred method.

	5. Make apache run at boot with your preferred method.

	6. Set the time correctly on your server or you might accidentally 
		call people at 4 in the morning or something.

## if you are running Rocky or Centos/RH
install mlocate, nlohmann-json-devel

Go to the MySQL Connector/C++ download page and download the latest version of the source code.
```bash
tar -xzvf mysql-connector-c++-8.x.xx.tar.gz
cd mysql-connector-c++-8.x.xx
```
```bash
mkdir build
cd build
cmake .. 
make
sudo make install
```