

```bash
docker run -d --name my-asterisk -p 5160:5060/udp -p 5161:5061/tcp -p 5162:5062/udp  -p 8188:8088/tcp  -p 20001-30000:10000-19999/udp  mlan/asterisk
```