install mysql version >= 8
asterisk version 18

set ODBC. Example:
`/etc/odbc.ini `
```ini
[asterisk]
Description = MySQL database for Asterisk
Driver      = MySQL
Database    = dialer
Server      = 127.0.0.1
User        = dialer
Password    = <Your DB pass>
Port        = 3306

```
`/etc/odbcinst.ini`
```ini
[MySQL]
Driver=/usr/lib/x86_64-linux-gnu/odbc/libmaodbc.so
Description=MariaDB Connector/ODBC(Unicode)
Threading=0
UsageCount=1
FileUsage= 1
```