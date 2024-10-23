```bash
cat ../gd_api/storage/logs/laravel.log | sed '/^Event: RTCPReceived/,/^$/d' | sed '/^Event: RTCPSent/,/^$/d'
```