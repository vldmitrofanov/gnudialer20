To configure Asterisk to work with PostgreSQL on Ubuntu, you’ll need to install PostgreSQL, configure it for Asterisk, and then set up Asterisk to use PostgreSQL for RealTime database storage.

Step 1: Install PostgreSQL
First, install PostgreSQL on your Ubuntu system.

```bash
sudo apt update
sudo apt install postgresql postgresql-contrib
```
Step 2: Configure PostgreSQL
Start and Enable PostgreSQL:
Ensure that PostgreSQL is running and will start on boot:

```bash
sudo systemctl start postgresql
sudo systemctl enable postgresql
```
### Create a PostgreSQL User for Asterisk:
By default, PostgreSQL uses a system called peer authentication, where PostgreSQL usernames must match the system usernames. We’ll create a PostgreSQL user for Asterisk.

### Switch to the postgres user:

```bash
sudo -i -u postgres
```
### Create a new PostgreSQL role:

```bash
createuser --interactive
```
Follow the prompts to create a new user. For example:

Enter name of role to add: asterisk
Shall the new role be a superuser? n
Shall the new role be allowed to create databases? n
Shall the new role be allowed to create more new roles? n
### Set a password for the new role:

```bash
psql
ALTER USER asterisk WITH PASSWORD 'your_password';
```
Create a new database for Asterisk:

```bash
createdb -O asterisk asterisk
```

or

```sql
CREATE DATABASE asterisk OWNER asterisk;
```
Exit the PostgreSQL shell:

```bash
\q
exit
```
Step 3: Configure Asterisk to Use PostgreSQL
Install the Asterisk PostgreSQL Module:
If you compiled Asterisk from source, ensure that you have installed the PostgreSQL dependencies:

```bash
sudo apt install libpq-dev
```
Then, you can install the res_config_pgsql module:

```bash
cd /path/to/asterisk/source
make menuselect
```
In menuselect, navigate to Resource Modules and ensure res_config_pgsql is selected (marked with [*]).

Then, recompile and install Asterisk:

```bash
make
sudo make install
```
Configure `res_config_pgsql.conf`:
You need to configure Asterisk to connect to the PostgreSQL database.

Edit `res_config_pgsql.conf`:

```bash
sudo nano /etc/asterisk/res_config_pgsql.conf
```
Add the following configuration:

```ini
[general]
dbhost = 127.0.0.1
dbport = 5432
dbname = asterisk
dbuser = asterisk
dbpass = your_password
requirements = warn  ; Change to 'fatal' to prevent Asterisk from starting if the connection fails
```
### Configure RealTime for Asterisk:
To use PostgreSQL as a RealTime backend, you need to configure the RealTime settings.

Edit extconfig.conf:

```bash
sudo nano /etc/asterisk/extconfig.conf
```
Add or modify the RealTime configurations:

```ini
[settings]
sipusers => pgsql,asterisk,sip_buddies
sippeers => pgsql,asterisk,sip_buddies
extensions => pgsql,asterisk,extensions_table
voicemail => pgsql,asterisk,voicemail_table
```
This configuration tells Asterisk to look up SIP users, SIP peers, extensions, and voicemail settings in the PostgreSQL database.

Step 4: Set Up RealTime Tables in PostgreSQL
You’ll need to create the necessary tables in the PostgreSQL database to store the RealTime data. Here is an example SQL script to create some typical RealTime tables:

```sql
CREATE TABLE sip_buddies (
    id SERIAL PRIMARY KEY,
    name VARCHAR(80) NOT NULL,
    host VARCHAR(31) DEFAULT 'dynamic',
    context VARCHAR(80),
    ipaddr VARCHAR(45),
    port INT,
    regseconds INT,
    accountcode VARCHAR(20),
    amaflags VARCHAR(7),
    callgroup VARCHAR(10),
    callerid VARCHAR(80),
    canreinvite CHAR(3),
    disallow VARCHAR(100),
    allow VARCHAR(100),
    dtmfmode VARCHAR(7),
    lastms INT,
    username VARCHAR(80),
    secret VARCHAR(80)
);

CREATE TABLE extensions_table (
    id SERIAL PRIMARY KEY,
    context VARCHAR(80),
    exten VARCHAR(80),
    priority INT,
    app VARCHAR(80),
    appdata VARCHAR(255)
);

CREATE TABLE voicemail_table (
    uniqueid SERIAL PRIMARY KEY,
    customer_id INT,
    context VARCHAR(80),
    mailbox VARCHAR(80),
    password VARCHAR(80),
    fullname VARCHAR(80),
    email VARCHAR(80),
    pager VARCHAR(80),
    tz VARCHAR(10),
    attach VARCHAR(4),
    saycid VARCHAR(4),
    dialout VARCHAR(10),
    callback VARCHAR(10),
    review VARCHAR(4),
    operator VARCHAR(4),
    envelope VARCHAR(4),
    sayduration VARCHAR(4),
    saydurationm INTEGER,
    sendvoicemail VARCHAR(4),
    delete VARCHAR(4),
    nextaftercmd VARCHAR(4),
    forcereset VARCHAR(4),
    voicemailtimeout INTEGER
);
```
Connect to PostgreSQL and execute the SQL script:
```bash
psql -h 127.0.0.1 -U asterisk -d asterisk -f /path/to/your/sql/script.sql
```
Step 5: Restart and Test Asterisk
Restart Asterisk:

```bash
sudo systemctl restart asterisk
```
Check for Connection Success:
Monitor the logs to ensure that Asterisk successfully connects to the PostgreSQL database without errors:

```bash
tail -f /var/log/asterisk/full
```
Summary
PostgreSQL Installation: Install PostgreSQL and set up a database and user for Asterisk.
Asterisk Configuration: Configure Asterisk to use PostgreSQL via the res_config_pgsql module.
RealTime Tables: Create the necessary RealTime tables in the PostgreSQL database.
Test: Restart Asterisk and ensure that it connects to the PostgreSQL database successfully.
This setup allows Asterisk to use PostgreSQL as a RealTime database, providing a robust and scalable backend for storing configuration data.