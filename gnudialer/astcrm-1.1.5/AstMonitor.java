//
// astcrm - General purpose CRM
//
// General purpose CRM software for asterisk
//
// Copyright (C) 2005, GnuDialer Project
//
// Heath Schultz <heath1444@yahoo.com>
// Richard Lyman <richard@dynx.net>
//
// This program is free software, distributed under the terms of
// the GNU General Public License.
//

// Conversion to commandline
//   uncomment main function
//   comment out showDocument
//   comment out AstConnect in paint
//   hardcode baseurl
//   make AstConnect static
import java.awt.*;
import java.awt.event.*;
import java.awt.Graphics;
import java.awt.Event;
import java.io.*;
import java.net.*;
import java.net.URL;
//import java.applet.AppletContext;
import java.applet.*;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.util.GregorianCalendar;
import java.lang.String;
import java.lang.Object;
import javax.swing.*;
//import netscape.javascript.*;

public class AstMonitor extends java.applet.Applet implements ActionListener, Runnable {
	boolean keepTrying = true;

        public static final String spause   	= " pause ";
	public static final String sunpause 	= "unpause";

        public static final String smonitor     = "monitor";
        public static final String sunmonitor   = "stopmon";

        String message, disposition, transfer, campaign, leadid, channel;

	Thread astThread;

        Button callback 	= new Button("0 Callback");
        Button noanswer 	= new Button("2 NoAnswer");
        Button voicemail 	= new Button("3 AnsMach ");
        Button busy 		= new Button("4 Busy    ");
        Button fbusy 		= new Button("5 FastBusy");
        Button faxmach 		= new Button("6 Fax Mach");
        Button disco 		= new Button("7 Disco   ");
        Button dnc 		= new Button("8 DNC     ");
        Button invalid 		= new Button("9 Invalid ");
        Button other 		= new Button("10 Other  ");
        Button nosale 		= new Button("11 No Sale");
        Button sale             = new Button("12 Sale   ");

        Button blank1           = new Button("          ");
	Button blank2           = new Button("          ");

	Button pause		= new Button(spause);
        Button monitor          = new Button(smonitor);

        Button hangup           = new Button("hangup");
        Button logout           = new Button("logout");
        //Button update           = new Button("update");
        Button blank3           = new Button("          ");
        Button blank4           = new Button("          ");
        Button blank5           = new Button("          ");
	Button blank6           = new Button("          ");
        Button blank7           = new Button("          ");
        Button blank8           = new Button("          ");
        Button blank9           = new Button("          ");
        Button blank10          = new Button("          ");
        Button blank11          = new Button("          ");
        Button blank12          = new Button("          ");
        Button blank13          = new Button("          ");
        Button blank14          = new Button("          ");

//        Button sched_cb         = new Button("Sched CB");
                
        
	public void init() {
                resize(150,475);

                setForeground( java.awt.Color.black );
                setBackground( new java.awt.Color(237,237,237) );

                setFont( new Font("Tacoma", Font.PLAIN, 12) );

                message = ">> message area <<";

		setLayout(new GridLayout(15, 2));

                callback.setActionCommand("callback");
                callback.addActionListener(this);
                this.add(callback);
                noanswer.setActionCommand("noanswer");
                noanswer.addActionListener(this);
                this.add(noanswer);
                voicemail.setActionCommand("voicemail");
                voicemail.addActionListener(this);
                this.add(voicemail);
                busy.setActionCommand("busy");
                busy.addActionListener(this);
                this.add(busy);
                fbusy.setActionCommand("fbusy");
                fbusy.addActionListener(this);
                this.add(fbusy);
                faxmach.setActionCommand("faxmach");
                faxmach.addActionListener(this);
                this.add(faxmach);
                disco.setActionCommand("disco");
                disco.addActionListener(this);
                this.add(disco);
                dnc.setActionCommand("dnc");
                dnc.addActionListener(this);
                this.add(dnc);
                invalid.setActionCommand("invalid");
                invalid.addActionListener(this);
                this.add(invalid);
                other.setActionCommand("other");
                other.addActionListener(this);
                this.add(other);
                nosale.setActionCommand("nosale");
                nosale.addActionListener(this);
                this.add(nosale);
                sale.setActionCommand("sale");
                sale.addActionListener(this);
                this.add(sale);
                
		this.add(blank1);
		blank1.setVisible( false );
		this.add(blank2);
                blank2.setVisible( false );
		
		pause.setActionCommand("pause");
		pause.addActionListener(this);
		this.add(pause);

		monitor.setActionCommand("monitor");
		monitor.addActionListener(this);
		this.add(monitor);

		hangup.setActionCommand("hangup");
                hangup.addActionListener(this);
                this.add(hangup);

                logout.setActionCommand("logout");
		logout.addActionListener(this);
		this.add(logout);

                //update.setActionCommand("update");
                //update.addActionListener(this);
                //this.add(update);

                this.add(blank3);
		blank3.setVisible( false );
                this.add(blank4);
                blank4.setVisible( false );
                this.add(blank5);
                blank5.setVisible( false );
                this.add(blank6);
                blank6.setVisible( false );
                this.add(blank7);
                blank7.setVisible( false );
                this.add(blank8);
                blank8.setVisible( false );
                this.add(blank9);
                blank9.setVisible( false );
                this.add(blank10);
                blank10.setVisible( false );
                this.add(blank11);
                blank11.setVisible( false );
                this.add(blank12);
                blank12.setVisible( false );
                this.add(blank13);
                blank13.setVisible( false );
                this.add(blank14);
                blank14.setVisible( false );
                                
//                sched_cb.setActionCommand("sched_cb");
//                sched_cb.addActionListener(this);
//                this.add(sched_cb);
                                
		astThread = new Thread(this);
		astThread.start();

	}

   void addButton(Container p, String name) {
          // Create a button with the given name and add it
          // to the given panel.  Set up the button to send
          // events to the applet.
      JButton b = new JButton(name);
      p.add(b);
      b.addActionListener(this);
   }
      

   void addButton(JPanel p, String name, Object option) {
          // Same as above, but use the "option" object
          // as an additional parameter in the add method.
      JButton b = new JButton(name);
      p.add(b, option);
      b.addActionListener(this);
   }

	public void destroy() {
		keepTrying = false;
		astThread = null;
		actionSocket("CRM_AgentLogoff",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));

	}

	public void run() {
		keepTrying = true;
		while(keepTrying) {
			AstConnect(getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
			// If we lose our connection or it fails, let's not hammer on asterisk.
			try {
				message = "Conn - Failed";
				repaint();
				Thread.sleep(3000);
				message = "Conn - Retry";
				repaint();
			}
			catch (Exception e) {}
			repaint();
		}
	}

	public void actionPerformed(ActionEvent e) {
		String command = e.getActionCommand();
		String thebase = "http://" + getCodeBase().getHost();

		if (command.equals("pause")) {
			if (pause.getLabel().compareTo(sunpause) == 0) {
				pause.setLabel(spause);
				actionSocket("MUE_AgentUnPause",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
                                message = "Ready, Awaiting Call";
			} else {
                                pause.setLabel(sunpause);
				actionSocket("MUE_AgentPause",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
                                message = "Paused, (still online)";
			}
			repaint();
		}

		if (command.equals("monitor")) {
			if (monitor.getLabel().compareTo(sunmonitor) == 0) {
				monitor.setLabel(smonitor);
				actionSocket("MUE_AgentUnMonitor",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
                                //message = "Ready, Awaiting Call";
			} else {
                                monitor.setLabel(sunmonitor);
				actionSocket("MUE_AgentMonitor",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
                                //message = "Paused, (still online)";
			}
			repaint();
		}

                if (command.equals("logout")) {
                        getAppletContext().showDocument(String2Url(thebase + "/crm/crmlogin.html"),"_parent");
			actionSocket("Logoff",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
			stop();
		}

                 if (command.equals("hangup")) {
			actionSocket("CRM_CallHangup",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
			hangup.setVisible( false );
			pause.setVisible( false );
                        //update.setVisible( false );
			monitor.setVisible( false );
			repaint();
		}


                //if (command.equals("update")) {
			//actionSocket("CRM_UpdateRecord",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
                        //update.setVisible( false );
                        //logout.setVisible( true );
                        //pause.setVisible( true );
                        //repaint();
		//}

                if (command.equals("callback") ||command.equals("noanswer") ||command.equals("voicemail") ||command.equals("busy") ||command.equals("fbusy") ||command.equals("faxmach") ||command.equals("disco") ||command.equals("dnc") ||command.equals("invalid") ||command.equals("other") ||command.equals("nosale") ||command.equals("sale")) {
			if (command.equals("callback"))
                		disposition = "0";
                        if (command.equals("noanswer"))
                                disposition = "2";
                        if (command.equals("voicemail"))
                                disposition = "3";
                        if (command.equals("busy"))
                                disposition = "4";
                        if (command.equals("fbusy"))
                                disposition = "5";
                        if (command.equals("faxmach"))
				disposition = "6";
                        if (command.equals("disco"))
                                disposition = "7";
                        if (command.equals("dnc"))
                                disposition = "8";
                        if (command.equals("invalid"))
                                disposition = "9";
                        if (command.equals("other"))
                                disposition = "10";
                        if (command.equals("nosale"))
                                disposition = "11";
                        if (command.equals("sale"))
                                disposition = "12";

			actionSocket("CRM_DispoRecord",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
                        //update.setVisible( false );
                        logout.setVisible( true );
                        pause.setVisible( true );
                        monitor.setVisible( false );
			hangup.setVisible( false );
                        callback.setVisible( false );
                        noanswer.setVisible( false );
                        voicemail.setVisible( false );
                        busy.setVisible( false );
                        fbusy.setVisible( false );
                        faxmach.setVisible( false );
                        disco.setVisible( false );
                        dnc.setVisible( false );
                        invalid.setVisible( false );
                        other.setVisible( false );
                        nosale.setVisible( false );
                        sale.setVisible( false );
                        repaint();
		}

                if (command.equals("sched_cb")) {
			actionSocket("CRM_Schedule_Callback",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
			//hangup.setVisible( false );
			//pause.setVisible( false );
                        //update.setVisible( false );
			//monitor.setVisible( false );
			//repaint();
		}


	}

	public void paint(Graphics g) {
		g.setColor( Color.black );
                g.drawString(" " + message,0,250);
                //showStatus("yadda yadda something text here");
                //hideStatus();
	}

	public void actionSocket(String theAction, boolean tf, String host, String agent, String manuser, String manpass) {

                Socket astActionSocket = null;
                DataOutputStream osA = null;
                BufferedReader isA = null;

                try {
                        //astActionSocket = new Socket(getCodeBase().getHost(), 5038);
                        astActionSocket = new Socket(host, 5038);
                        osA = new DataOutputStream(astActionSocket.getOutputStream());
                        isA = new BufferedReader(new InputStreamReader(astActionSocket.getInputStream()));
                }
                catch (UnknownHostException e) {
                        System.err.println("actionSocket - Unknown host: " + e);
                }
                catch (IOException e) {
                        System.err.println("actionSocket - IOException: " + e);
                }

		if (astActionSocket != null && osA != null && isA != null) {
			try {
				// this opens the manager session
				osA.writeBytes("Action: Login\r\n");
                        	osA.writeBytes("UserName: " + manuser + "\r\n");
                        	osA.writeBytes("Secret: " + manpass + "\r\n\r\n");
				String resp, block = new String();

				// handle all the events here

	                        if (theAction == "Logoff") {
                                        osA.writeBytes("Action: Agentlogoff\r\n");
					osA.writeBytes("Agent: " + agent + "\r\n\r\n");

                         	}

				if (theAction == "AgentLogoff") {
					osA.writeBytes("Action: Agentlogoff\r\n");
					osA.writeBytes("Agent: " + agent + "\r\n\r\n");
				}

                                if (theAction == "AgentPause") {
                                        osA.writeBytes("Action: AgentPause\r\n");
                                        osA.writeBytes("Agent: " + agent + "\r\n");
                                        if (tf) {
	                                        osA.writeBytes("Pause: true\r\n\r\n");
					} else {
                                                osA.writeBytes("Pause: false\r\n\r\n");
					}
                                }

                                if (theAction == "MUE_AgentPause") {
                                        osA.writeBytes("Action: sendevent\r\n");
                                        osA.writeBytes("Event: AgentPause\r\n");
                                        osA.writeBytes("Body: Agent: " + agent + "\r\n\r\n");
                                }

                                if (theAction == "MUE_AgentUnPause") {
					osA.writeBytes("Action: sendevent\r\n");
					osA.writeBytes("Event: AgentUnPause\r\n");
					osA.writeBytes("Body: Agent: " + agent + "\r\n\r\n");
				}

                                if (theAction == "MUE_AgentMonitor") {
                                        osA.writeBytes("Action: sendevent\r\n");
                                        osA.writeBytes("Event: AgentMonitor\r\n");
                                        osA.writeBytes("Body: Agent: " + agent + "|Campaign: " + campaign + "|Leadid: " + leadid + "|Channel: " + channel +"\r\n\r\n");
                                }

                                if (theAction == "MUE_AgentUnMonitor") {
					osA.writeBytes("Action: sendevent\r\n");
					osA.writeBytes("Event: AgentUnMonitor\r\n");
					osA.writeBytes("Body: Agent: " + agent + "|Campaign: " + campaign + "|Leadid: " + leadid + "|Channel: " + channel +"\r\n\r\n");
				}

                                if (theAction == "CRM_AgentLogoff") {
                                        osA.writeBytes("Action: sendevent\r\n");
                                        osA.writeBytes("Event: CRM_AgentLogoff\r\n");
                                        osA.writeBytes("Body: Agent: " + agent + "\r\n\r\n");
                                }

                                if (theAction == "CRM_LoginAgent") {
					osA.writeBytes("Action: sendevent\r\n");
					osA.writeBytes("Event: CRM_LoginAgent\r\n");
					osA.writeBytes("Body: Agent: " + agent + "\r\n\r\n");
				}

                                if (theAction == "CRM_CallHangup") {
                                        osA.writeBytes("Action: sendevent\r\n");
                                        osA.writeBytes("Event: CRM_CallHangup\r\n");
                                        osA.writeBytes("Body: Agent: " + agent + "|Campaign: " + campaign + "|Leadid: " + leadid + "|Channel: " + channel +"\r\n\r\n");
                                }

                                if (theAction == "CRM_UpdateRecord") {
                                        osA.writeBytes("Action: sendevent\r\n");
                                        osA.writeBytes("Event: CRM_UpdateRecord\r\n");
                                        osA.writeBytes("Body: Agent: " + agent + "\r\n\r\n");
                                }
                                if (theAction == "CRM_DispoRecord") {
                                        osA.writeBytes("Action: sendevent\r\n");
                                        osA.writeBytes("Event: CRM_DispoRecord\r\n");
                                        osA.writeBytes("Body: Agent: " + agent + "|Dispo: " + disposition + "|Transfer: " + transfer + "|Campaign: " + campaign + "|Leadid: " + leadid + "|Channel: " + channel +"\r\n\r\n");
				}

                                if (theAction == "CRM_Schedule_Callback") {
                                        osA.writeBytes("Action: sendevent\r\n");
                                        osA.writeBytes("Event: CRM_Schedule_Callback\r\n");
                                        osA.writeBytes("Body: Agent: " + agent + "|Dispo: " + disposition + "|Transfer: " + transfer + "|Campaign: " + campaign + "|Leadid: " + leadid + "|Channel: " + channel +"\r\n\r\n");
				}

                                // this closes the manager session
                                //osA.writeBytes("Action: Logoff\r\n\r\n");
				//osA.close();
				//isA.close();
				//astActionSocket.close();

                                if (theAction == "CRM_CallHangup") {
                                        pause.setLabel(sunpause);
        	                        monitor.setLabel(sunmonitor);
	                                actionSocket("MUE_AgentPause",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
		                        pause.setVisible( true );
                	                message = "Paused, In Closeout";
                	                repaint();
				}
                                if (theAction == "CRM_DispoRecord") {
					pause.setLabel(spause);
                                        monitor.setLabel(sunmonitor);
					actionSocket("MUE_AgentUnPause",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));
		                        pause.setVisible( true );
                	                message = "Ready, Awaiting Call";
                	                repaint();
				}

			}

			catch (UnknownHostException e) {
				System.err.println("actionSocket - Unknown host: " + e);
			}

			catch (IOException e) {
				System.err.println("actionSocket - IOException: " + e);
			}

			finally {
				try {
					osA.close();
					isA.close();
					astActionSocket.close();
				}
				catch (IOException e) {
					System.err.println("actionSocket - IOException: " + e);
				}
			}
		}
	}

	public void AstConnect(String host, String agent, String manuser, String manpass) {

		Socket AstSocket = null;
		DataOutputStream os = null;
		BufferedReader is = null;

		try {
                    //AstSocket = new Socket(getCodeBase().getHost(), 5038);
                    AstSocket = new Socket(host, 5038);
			os = new DataOutputStream(AstSocket.getOutputStream());
			is = new BufferedReader(new InputStreamReader(AstSocket.getInputStream()));
		}
		catch (UnknownHostException e) {
                        System.err.println("AstConnect - Unknown host: " + e);
			//System.err.println("Error finding host: hostname");
		}
		catch (IOException e) {
                        //System.err.println("Error sending/receiving data to/from: hostname");
                        System.err.println("AstConnect - IOException: " + e);
		}

		if (AstSocket != null && os != null && is != null) {
			try {
				os.writeBytes("Action: Login\r\n");
				os.writeBytes("UserName: " + manuser + "\r\n");
				os.writeBytes("Secret: " + manpass + "\r\n\r\n");
				String resp, block = new String();
				//String campaign = new String();
				//String leadid = new String();
				campaign = "";
				leadid = "";
				channel = "";
				String flushscreen = new String();
				//int isPort = getPort();
				int isPort = getCodeBase().getPort();
				String sUrl = new String();			
				String baseurl, thebase;

//Added by Jamerson to fix URL with :PORT 

//	if (isPort == 0) { 
		baseurl = "http://" + host + "/cgi-bin/crm.cgi";
                thebase = "http://" + host;
//	} else { 
//		baseurl = "http://" + host + ":" + getCodeBase().getPort() + "/cgi-bin/crm.cgi";
//		thebase = "http://" + host + ":" + getCodeBase().getPort();
//	}

//String sUrl = new String();
//Old for ports
//String baseurl = "http://" + getCodeBase().getHost() + ":" + getCodeBase().getPort() + "/cgi-bin/crm.cgi";
//String thebase = "http://" + getCodeBase().getHost() + ":" + getCodeBase().getPort();
//Old w/p ports
//String baseurl = "http://" + host + "/cgi-bin/crm.cgi";
//String thebase = "http://" + host;

                        	int pos = 0;
                        	int end = 0;

                        	while ((resp = is.readLine()) != null) {

                        		if (resp.indexOf(":") != -1 || resp.indexOf("Agent") != -1)
                        			block += resp + "\n";
					else {

						if (block.indexOf("Response: Success",0) != -1) {
							//logged in trigger
							actionSocket("CRM_LoginAgent",true,getParameter("thehost"),getParameter("station"),getParameter("manageruser"),getParameter("managerpass"));

                                                	callback.setVisible( false );
                                                	noanswer.setVisible( false );
                                                	voicemail.setVisible( false );
                                                	busy.setVisible( false );
                                                	fbusy.setVisible( false );
                                                	faxmach.setVisible( false );
                                                	disco.setVisible( false );
                                                	dnc.setVisible( false );
                                                	invalid.setVisible( false );
                                                	other.setVisible( false );
                                                	nosale.setVisible( false );
                                                	sale.setVisible( false );
							pause.setVisible( false );
                                                        monitor.setVisible( false );
							logout.setVisible( false );
							hangup.setVisible( false );
							//update.setVisible( false );
							message = "Also Login on Phone";
							repaint();

							getAppletContext().showDocument(String2Url(baseurl + "?campaign=-1&leadid=0&agent=0&station=0"),"actualcrm");
							try {
								Thread.sleep(500);
							}
							catch (Exception e) {}
								getAppletContext().showDocument(String2Url(baseurl + "?campaign=-2&leadid=0&agent=0&station=0"),"actualcrm");

						}

						if (block.indexOf("Goodbye",0) != -1) {
							os.close();
							is.close();
							AstSocket.close();

							keepTrying = false;
//							stop();
						}

						if (block.indexOf("Event: ManagerUserEvent") != -1 && block.indexOf("Event: CRM_LoginAgent") != -1 && block.indexOf("Agent: " + agent + "\n") != -1) {
          						logout.setVisible( true );
          						repaint();
						}

						if (block.indexOf("Event: ManagerUserEvent") != -1 && block.indexOf("Event: CRM_UpdateRecord") != -1 && block.indexOf("Agent: " + agent + "\n") != -1) {
							sUrl = baseurl +
							"?campaign=" + -2 +
							"&monitor=" + getParameter("monitor") +
							"&leadid=" + 0 +
							"&agent=" + agent +
							"&station=1";

							getAppletContext().showDocument(String2Url(sUrl),"actualcrm");

                                                        callback.setVisible( false );
                                                        noanswer.setVisible( false );
                                                        voicemail.setVisible( false );
                                                        busy.setVisible( false );
                                                        fbusy.setVisible( false );
                                                        faxmach.setVisible( false );
                                                        disco.setVisible( false );
                                                        dnc.setVisible( false );
                                                        invalid.setVisible( false );
                                                        other.setVisible( false );
                                                        nosale.setVisible( false );
                                                        sale.setVisible( false );
							//update.setVisible( false );
							message = "Ready, Awaiting Call";

							repaint();
						}

						if (block.indexOf("Event: ManagerUserEvent") != -1 && block.indexOf("Event: CRM_DispoRecord") != -1 && block.indexOf("Agent: " + agent + "|") != -1) {
							sUrl = baseurl +
							"?campaign=" + -2 +
							"&monitor=" + getParameter("monitor") +
							"&leadid=" + 0 +
							"&agent=" + agent +
							"&station=1";

							getAppletContext().showDocument(String2Url(sUrl),"actualcrm");

                                                        callback.setVisible( false );
                                                        noanswer.setVisible( false );
                                                        voicemail.setVisible( false );
                                                        busy.setVisible( false );
                                                        fbusy.setVisible( false );
                                                        faxmach.setVisible( false );
                                                        disco.setVisible( false );
                                                        dnc.setVisible( false );
                                                        invalid.setVisible( false );
                                                        other.setVisible( false );
                                                        nosale.setVisible( false );
                                                        sale.setVisible( false );
							//update.setVisible( false );
							monitor.setLabel(smonitor);
                                                        monitor.setVisible( false );
							message = "Ready, Awaiting Call";

							repaint();
						}

						if (block.indexOf("Event: UserEventDispo") != -1 && block.indexOf("Agent: " + agent + "|") != -1) {
							sUrl = baseurl +
							"?campaign=" + -2 +
							"&monitor=" + getParameter("monitor") +
							"&leadid=" + 0 +
							"&agent=" + agent +
							"&station=1";

							getAppletContext().showDocument(String2Url(sUrl),"actualcrm");

                                                        callback.setVisible( false );
                                                        noanswer.setVisible( false );
                                                        voicemail.setVisible( false );
                                                        busy.setVisible( false );
                                                        fbusy.setVisible( false );
                                                        faxmach.setVisible( false );
                                                        disco.setVisible( false );
                                                        dnc.setVisible( false );
                                                        invalid.setVisible( false );
                                                        other.setVisible( false );
                                                        nosale.setVisible( false );
                                                        sale.setVisible( false );
							//update.setVisible( false );
							logout.setVisible( true );
                                                        hangup.setVisible( false );
				                        pause.setVisible( true );
				                        monitor.setLabel(smonitor);
                                                        monitor.setVisible( false );
                                                        message = "Ready, Awaiting Call";

							repaint();
						}

						if ((block.indexOf("Event: Agentlogoff") != -1 || block.indexOf("Event: AgentLogoff") != -1) && block.indexOf("Agent: " + agent + "\n") != -1) {
							pause.setVisible( false );
							repaint();

							getAppletContext().showDocument(String2Url(thebase + "/crm/crmlogin.html"),"_parent");

							os.writeBytes("Action: Logoff\r\n\r\n");
//							stop();
						}

						if ((block.indexOf("Event: Agentlogin") != -1) && (block.indexOf("Agent: " + agent + "\n") != -1)) {
							pause.setVisible( true );
							message = "Ready, Awaiting Call";
							repaint();
						}

						if ((block.indexOf("Event: UserEventAgentCalled") != -1 || block.indexOf("Event: UserEventClserCalled") != -1) && block.indexOf("Agent/" + agent + " ") != -1) {
							flushscreen = block;
	        	                                if (block.indexOf("Channel: ") != -1) {
        	        	                                pos = block.indexOf("Channel: ");
                	        	                        end = block.indexOf("\n",pos);
                        	        	                channel = block.substring(pos+9,end);
							}

							pos = block.indexOf("~");
							end = block.indexOf("-",pos);
							campaign = block.substring(pos+1,end);

							if (block.indexOf("UserEventClserCalled") != -1) {
								campaign += "-isclosercam";
								transfer = "";
							} else {
								transfer = "TRANSFER";
							}

							pos = end+1;
							end = block.indexOf("-",pos);
							leadid = block.substring(pos,end);

//							campaign = "test1";
//							leadid = "5000";

							sUrl = baseurl +
							"?campaign=" + campaign +
							"&monitor=" + getParameter("monitor") +
							"&leadid=" + leadid +
							"&agent=" + agent +
							"&station=1";

							// this is to allow time for the agents dispo and call info to
							// write to the database before grabbing the info for the closer
							if (block.indexOf("ClserCalled") != -1) {
	                                        		try {
        	                                        		Thread.sleep(250);
								}
								catch (Exception e) {}
							}

							getAppletContext().showDocument(String2Url(sUrl),"actualcrm");

                	                                callback.setVisible( true );
                        	                        noanswer.setVisible( true );
                                	                voicemail.setVisible( true );
                                        	        busy.setVisible( true );
                                                	fbusy.setVisible( true );
              	        	                        faxmach.setVisible( true );
                	                                disco.setVisible( true );
	                                                dnc.setVisible( true );
        	                                        invalid.setVisible( true );
                	                                other.setVisible( true );
                        	                        nosale.setVisible( true );
                                	                sale.setVisible( true );

							logout.setVisible( false );
							hangup.setVisible( true );
                                                	//update.setVisible( true ); 
							pause.setVisible( false );
							monitor.setLabel(smonitor);
                                                        monitor.setVisible( true );
							message = "Connect, Awaiting Dispo";
							repaint();
						}

						if (block.indexOf("Unlink") != -1 && block.indexOf("Agent/" + agent + "") != -1 && flushscreen.indexOf("true") != -1) {
                                        		hangup.setVisible( false );
                                        		repaint();
            					}

						if (block.indexOf("Event: ManagerUserEvent") != -1 && block.indexOf("Event: CRM_Schedule_Callback") != -1 && block.indexOf("Agent: " + agent + "|") != -1) {
							//sUrl = baseurl +
							//"?campaign=" + -2 +
							//"&monitor=" + getParameter("monitor") +
							//"&leadid=" + 0 +
							//"&agent=" + agent +
							//"&station=1";

							//getAppletContext().showDocument(String2Url(sUrl),"actualcrm");
                                                        getAppletContext().showDocument(String2Url("http://prophet.dynx.net/crm/simple.html"),"_blank");
                                                        
                                                        //callback.setVisible( false );
                                                        //noanswer.setVisible( false );
                                                        //voicemail.setVisible( false );
                                                        //busy.setVisible( false );
                                                        //fbusy.setVisible( false );
                                                        //faxmach.setVisible( false );
                                                        //disco.setVisible( false );
                                                        //dnc.setVisible( false );
                                                        //invalid.setVisible( false );
                                                        //other.setVisible( false );
                                                        //nosale.setVisible( false );
                                                        //sale.setVisible( false );
							//update.setVisible( false );
							//monitor.setLabel(smonitor);
                                                        //monitor.setVisible( false );
							//message = "Ready, Awaiting Call";

							//repaint();
						}


	            				block = "";
        	    				sUrl = "";
					}

					if (resp.indexOf("DIALERQUIT") != -1) {
						break;
					}
				}

				os.close();
				is.close();
				AstSocket.close();
			}

			catch (UnknownHostException e) {
				System.err.println("AstConnect - Unknown host: " + e);
			}
			catch (IOException e) {
				System.err.println("AstConnect - IOException: " + e);
			}
		}
	}


	public static void GetDate(String[] args) {
        	SimpleDateFormat bartDateFormat = new SimpleDateFormat("EEEE-MMMM-dd-yyyy");
        	Date date = new Date();     
        	System.out.println(date.getTime());
        }


	public static URL String2Url(String theString) {

		URL theUrl = null;
		try {
			theUrl = new URL(theString);
			return theUrl;
		}
		catch (Exception e) {}
		return theUrl;

	}

}




