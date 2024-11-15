## here is the example of received bridge
{
  "variable": "BRIDGEPEER",
  "value": "",
  "type": "ChannelVarset",
  "timestamp": "2024-08-20T14:52:33.801-0700",
  "channel": {
    "id": "1724189586.226",
    "name": "PJSIP/10002-00000010",
    "state": "Up",
    "protocol_id": "duJug3LeDV9sIsh5ZrxJaw..",
    "caller": {
      "name": "Agent",
      "number": "10002"
    },
    "connected": {
      "name": "~test1-112-true~",
      "number": "9999999999"
    },
    "accountcode": "",
    "dialplan": {
      "context": "dialerlogin",
      "exten": "s",
      "priority": 7,
      "app_name": "",
      "app_data": ""
    },
    "creationtime": "2024-08-20T14:33:32.328-0700",
    "language": "en"
  },
  "asterisk_id": "02:42:c3:6d:d8:07",
  "application": "gnudialer_stasis_app"
}

## when channel left bridge
{
  "type": "ChannelLeftBridge",
  "timestamp": "2024-08-21T17:23:48.047-0700",
  "bridge": {
    "id": "00262442-00d0-41e8-9d16-7c0d4ecff244",
    "technology": "simple_bridge",
    "bridge_type": "mixing",
    "bridge_class": "basic",
    "creator": "",
    "name": "",
    "channels": [
      "1724286189.288"
    ],
    "creationtime": "2024-08-21T17:23:41.539-0700",
    "video_mode": "none"
  },
  "channel": {
    "id": "1724286211.289",
    "name": "SIP/shirker_net-0000005e",
    "state": "Up",
    "protocol_id": "4ea1becd0c483f6d0b1d8e1e7f574244@shirker.net",
    "caller": {
      "name": "~test1-115-true~",
      "number": "9999999999"
    },
    "connected": {
      "name": "Agent",
      "number": "10002"
    },
    "accountcode": "test1",
    "dialplan": {
      "context": "gdincoming",
      "exten": "talk",
      "priority": 6,
      "app_name": "",
      "app_data": ""
    },
    "creationtime": "2024-08-21T17:23:41.539-0700",
    "language": "en"
  },
  "asterisk_id": "02:42:c3:6d:d8:07",
  "application": "gnudialer_stasis_app"
}