

curl -X POST "http://localhost:8088/ari/channels?endpoint=PJSIP/faketrunk/sip:084457466886@127.0.0.1:5060&extension=084457466886&context=faketrunk-incoming&priority=1&callerId=CallerID" -u apiuser:secret123

curl -X POST "http://localhost:8088/ari/channels?endpoint=PJSIP/faketrunk/sip:084457466886@192.168.1.196&extension=s&context=dialermenu&priority=1&callerId=CallerID" -u apiuser:secret123