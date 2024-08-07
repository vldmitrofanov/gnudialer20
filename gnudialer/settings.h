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
#include <string>
#include <vector>
#include <fstream>
#include "etcinfo.h"
#include "queue.h"
                        
void addGlobalSettings(std::string context) {

        Queue TheQueueGlobals;
	TheQueueGlobals.ParseQueue(context);
        TheQueueGlobals.SupSetting("debug","true");
	TheQueueGlobals.SupSetting("log","true");
	TheQueueGlobals.Write();
}

void resetFilterSettings(std::string campaign) {

        Queue TheQueue;
        TheQueue.ParseQueue(campaign);

        TheQueue.SupSetting("f_mode","0");
        TheQueue.SupSetting("f_areacode","0");
        TheQueue.SupSetting("f_areacode_prefix","0");
        TheQueue.SupSetting("f_zipcode","0");

        for (int f = 0; f < TheQueue.OccurencesOf("filters"); f++) {
        	TheQueue.GetSetting(f,"filters").SupAttribute("enable","false");
        	//std::cout << campaign << ": Resetting Filter Number : " << f << std::endl;
	}
        TheQueue.GetSetting(0,"filters").SupAttribute("enable","true");
        TheQueue.Write();
}

                
void addBasicSettings(std::string campaign) {

        Queue TheQueue;
        TheQueue.ParseQueue(campaign);
        TheQueue.SupSetting("active","false");
        TheQueue.SupSetting("function","predictive");
	TheQueue.SupSetting("calltoday","false");
        TheQueue.SupSetting("usednc","false");
        TheQueue.SupSetting("maxratio","2");
        TheQueue.SupSetting("maxabandons","3.0");
        TheQueue.SupSetting("maxlines","23");
        TheQueue.SupSetting("sleeptime","1000");
        TheQueue.SupSetting("tzearliest","8");
        TheQueue.SupSetting("tzlatest","21");
        TheQueue.SupSetting("timeout","20000");
        TheQueue.SupSetting("dspmode","backgrounddetect");
	TheQueue.SupSetting("trunk","Zap/g1");
        TheQueue.SupSetting("dialprefix","none");
	TheQueue.SupSetting("usecloser","true");
        TheQueue.SupSetting("closercam","CLOSER");
	TheQueue.SupSetting("callerid","9999999999");
	TheQueue.SupSetting("filter","0");
        TheQueue.SupSetting("calls","0");
        TheQueue.SupSetting("abandons","0");
	TheQueue.SupSetting("totalcalls","0");
	TheQueue.SupSetting("totalabandons","0");
	TheQueue.SupSetting("talktime","0");
	TheQueue.SupSetting("totaltalktime","0");
        TheQueue.SupSetting("usecallback","false");
        TheQueue.SupSetting("usetzfilter","false");
        TheQueue.SupSetting("debug","true");
        TheQueue.SupSetting("skip","0");
        TheQueue.SupSetting("f_mode","0");
        TheQueue.SupSetting("f_areacode","0");
        TheQueue.SupSetting("f_areacode_prefix","0");
        TheQueue.SupSetting("f_zipcode","0");
        TheQueue.SupSetting("prn_agent_sales","false");
        TheQueue.SupSetting("prn_closer_sales","false");
        TheQueue.SupSetting("prn_closer_nosales","true");
        TheQueue.SupSetting("orderby","attempts_pickups");
         
        if (!TheQueue.SettingExists("filters","number","0")) {
        	TheQueue.AddSetting("filters","number:0:enable:true:string:((disposition > -6 AND disposition < 6) AND attempts < 4)");
	}
        if (!TheQueue.SettingExists("filters","number","1")) {
                TheQueue.AddSetting("filters","number:1:enable:false:string:((disposition > -6 AND disposition < 6) AND (attempts > 3 AND  attempts < 8))");
	}
        if (!TheQueue.SettingExists("filters","number","2")) {
                TheQueue.AddSetting("filters","number:2:enable:false:string:((disposition > -6 AND disposition < 6) AND attempts > 7)"); 
	}
        if (!TheQueue.SettingExists("filters","number","3")) {
                TheQueue.AddSetting("filters","number:3:enable:false:string:(disposition = -3 OR disposition = 3)");
	}
        if (!TheQueue.SettingExists("filters","number","4")) {
                TheQueue.AddSetting("filters","number:4:enable:false:string:(disposition = 0)");
	}
        if (!TheQueue.SettingExists("filters","number","5")) {
                TheQueue.AddSetting("filters","number:5:enable:false:string:(disposition = -7)");
	}


	//phone,      title,     name,       address,    city,       state,      zip,        comments,   countynum
	//varchar(15),varchar(5),varchar(40),varchar(40),varchar(20),varchar(15),varchar(10),varchar(60),varchar(5)

//        if (!TheQueue.SettingExists("fields","number","0")) {
//                TheQueue.AddSetting("fields","number:0:name:phone:type:varchar:size:15:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","1")) {
//                TheQueue.AddSetting("fields","number:1:name:title:type:varchar:size:5:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","2")) {
//                TheQueue.AddSetting("fields","number:2:name:name:type:varchar:size:40:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","3")) {
//                TheQueue.AddSetting("fields","number:3:name:address:type:varchar:size:40:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","4")) {
//                TheQueue.AddSetting("fields","number:4:name:city:type:varchar:size:20:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","5")) {
//                TheQueue.AddSetting("fields","number:5:name:state:type:varchar:size:15:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","6")) {
//                TheQueue.AddSetting("fields","number:6:name:zip:type:varchar:size:10:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","7")) {
//                TheQueue.AddSetting("fields","number:7:name:comments:type:varchar:size:60:export:true");
//                }
//        if (!TheQueue.SettingExists("fields","number","8")) {
//                TheQueue.AddSetting("fields","number:8:name:countynum:type:varchar:size:5:export:true");
//                }

        TheQueue.Write();        
}

//queue notes
//
//TheQueues.GetSetting(x,y).SetAttribute(field,value);
//returns 1 on success, 0 on fail because the attribute didn't already exists, and -1 (not quite sure what that would be)

//TheQueues.GetSetting(x,y).SupAttribute(field,value);
//returns 1 if successfully changed attribute, returns 2 if successfully replaced attribute, and -1 on error

//TheQueues.AddSetting("filters","number:5:enable:false");
//returns 1 on success, -1 on failure

//if (TheQueue.GetSetting(1,"filters").SetAttribute("enabled","false") != 1) {
//        std::cout << "Something went wrong disabling filter!" << std::endl;
//}


