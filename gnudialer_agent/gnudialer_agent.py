#!/usr/bin/env python3
#
# don't forget ro run
# pip install mysql-connector-python
#
import mysql.connector
import json
import telnetlib

# Read the /etc/gnudialer.conf file
def read_gnudialer_conf(conf_file):
    with open(conf_file, 'r') as file:
        lines = file.readlines()


    # Debug: Print out the lines read from the file
    for i, line in enumerate(lines):
        print(f"Line {i+1}: {line.strip()}")

    config = {
        "mysql_user": lines[1].strip(),     # 2nd line: MySQL user
        "mysql_pass": lines[2].strip(),     # 3rd line: MySQL password
        "mysql_host": lines[3].strip(),     # 4th line: MySQL host
        "mysql_db": lines[4].strip(),     # 4th line: MySQL host
        "mysql_port": int(lines[5].strip()),  # 5th line: MySQL port
        "ami_user": lines[6].strip(),       # 6th line: AMI user
        "ami_secret": lines[7].strip(),     # 7th line: AMI secret
        "ami_host": lines[8].strip(),       # 8th line: AMI host
        "server_id": int(lines[9].strip())   # 9th line: Server ID
    }
    return config

# Check if there are any queues with synced = 0
def check_unsynced(config):
    connection = mysql.connector.connect(
        host=config["mysql_host"],
        user=config["mysql_user"],
        password=config["mysql_pass"],
        database="gnudialer",
        port=config["mysql_port"]
    )
    cursor = connection.cursor()

    query = """
    SELECT COUNT(*)
    FROM servers
    WHERE servers.id = %s AND synced = 0;
    """
    
    cursor.execute(query, (config["server_id"],))
    count = cursor.fetchone()[0]

    connection.close()

    return count > 0

# Connect to MySQL and fetch the agents
def fetch_agents(config):
    connection = mysql.connector.connect(
        host=config["mysql_host"],
        user=config["mysql_user"],
        password=config["mysql_pass"],
        database=config["mysql_db"],
        port=config["mysql_port"]
    )
    cursor = connection.cursor(dictionary=True)

    # SQL query to select agents and their user details
    query = """
    SELECT 
        a.id as agent_id, u.first_name, u.last_name, a.password, a.settings
    FROM 
        agents a
    JOIN 
        users u ON a.user_id = u.id
    JOIN 
        agent_queue qa ON qa.agent_id = a.id
    WHERE 
        a.server_id = %s;
    """
    
    cursor.execute(query, (config["server_id"],))
    agents = cursor.fetchall()
    
    connection.close()
    return agents

# Write agents to gnudialer_agents.conf
def write_gnudialer_agents_and_sip_conf(agents, conf_file, sip_conf_file, server_id):
    with open(conf_file, 'w') as file, open(sip_conf_file, 'w') as sip_file:
        for agent in agents:
            agent_id = agent["agent_id"]
            agent_name = f"{agent['first_name']} {agent['last_name']}"
            agent_pass = agent["password"]
            formatted_server_id = f"{server_id:02}"
            extension = f"9{formatted_server_id}{agent_id}"

            file.write(f"agent => {agent_id},{agent_pass},{agent_name}\n")

            sip_settings = json.loads(agent["settings"]).get("sip", {})
            sip_file.write(f"[{extension}]\n")
            sip_file.write(f"type=friend\n")

            # Iterate over SIP settings and write key-value pairs
            for key, value in sip_settings.items():
                sip_file.write(f"{key}={value}\n")

            sip_file.write("\n")

# Fetch queues and return them as an array
def fetch_queues(config):
    connection = mysql.connector.connect(
        host=config["mysql_host"],
        user=config["mysql_user"],
        password=config["mysql_pass"],
        database=config["mysql_db"],
        port=config["mysql_port"]
    )
    cursor = connection.cursor(dictionary=True)

    # SQL query to select queues with their associated campaign details
    query = """
    SELECT 
        q.id as queue_id, c.code as queue_name, q.settings as queue_settings, c.id as campaign_id
    FROM 
        queues q
    JOIN 
        campaigns c ON q.campaign_id = c.id
    WHERE 
        q.server_id = %s AND q.status > 0;
    """
    
    cursor.execute(query, (config["server_id"],))
    queues = cursor.fetchall()
    
    connection.close()
    return queues

# Write queue configuration to gnudialer_queues.conf
def write_gnudialer_queues_conf(config, queues, conf_file):
    with open(conf_file, 'w') as file:
        for queue in queues:
            campaign_code = queue["queue_name"]
            queue_settings = json.loads(queue["queue_settings"])

            file.write(f"[{campaign_code}]\n")
            for setting, value in queue_settings.items():
                file.write(f"{setting} = {value}\n")

            # Fetch and write agents associated with this queue
            agents = fetch_agents_for_queue(config, queue["queue_id"])
            for agent_id in agents:
                file.write(f"member => Agent/{agent_id}\n")
            file.write("\n")

# Fetch agents associated with a specific queue
def fetch_agents_for_queue(config, queue_id):
    connection = mysql.connector.connect(
        host=config["mysql_host"],
        user=config["mysql_user"],
        password=config["mysql_pass"],
        database=config["mysql_db"],
        port=config["mysql_port"]
    )
    cursor = connection.cursor(dictionary=True)

    query = """
    SELECT 
        a.id as agent_id
    FROM 
        agent_queue qa
    JOIN 
        agents a ON qa.agent_id = a.id
    WHERE 
        qa.queue_id = %s;
    """

    cursor.execute(query, (queue_id,))
    agents = [row["agent_id"] for row in cursor.fetchall()]

    connection.close()
    return agents

# Update synced status in the database
def update_queues_synced_status(config):
    connection = mysql.connector.connect(
        host=config["mysql_host"],
        user=config["mysql_user"],
        password=config["mysql_pass"],
        database=config["mysql_db"],
        port=config["mysql_port"]
    )
    cursor = connection.cursor()

    update_query = """
    UPDATE 
        servers 
    SET 
        synced = 1 
    WHERE 
        servers.id = %s;
    """
    
    cursor.execute(update_query, (config["server_id"],))
    connection.commit()
    
    connection.close()

def ami_core_reload(config):
    # Connect to AMI
    try:
        tn = telnetlib.Telnet(config["ami_host"], 5038)  # Default AMI port is 5038

        # Login to AMI
        tn.write(b"Action: Login\r\n")
        tn.write(f"Username: {config['ami_user']}\r\n".encode('utf-8'))
        tn.write(f"Secret: {config['ami_secret']}\r\n".encode('utf-8'))
        tn.write(b"Events: off\r\n\r\n")

        # Execute core reload
        tn.write(b"Action: Command\r\n")
        tn.write(b"Command: core reload\r\n\r\n")

        # Logout from AMI
        tn.write(b"Action: Logoff\r\n\r\n")

        # Read all responses from AMI (optional, for debugging)
        response = tn.read_all().decode('utf-8')
        print("AMI Response:\n", response)

        tn.close()
    except Exception as e:
        print(f"Error connecting to AMI: {e}")

def main():
    config_file = "/etc/gnudialer.conf"
    gnudialer_agents_conf = "/etc/asterisk/gnudialer_agents.conf"
    gnudialer_queues_conf = "/etc/asterisk/gnudialer_queues.conf"
    gnudialer_sip_conf = "/etc/asterisk/gnudialer_sip.conf"

    config = read_gnudialer_conf(config_file)

    # Check if there are unsynced queues
    if not check_unsynced(config):
        print("No unsynced settings found. Exiting.")
        return

    # Fetch agents and write gnudialer_agents.conf
    agents = fetch_agents(config)
    write_gnudialer_agents_and_sip_conf(agents, gnudialer_agents_conf, gnudialer_sip_conf, config["server_id"])

    # Fetch queues and write gnudialer_queues.conf
    queues = fetch_queues(config)
    write_gnudialer_queues_conf(config, queues, gnudialer_queues_conf)

    # Update synced status in the database
    update_queues_synced_status(config)

    # reload the core
    ami_core_reload(config)

    print("Configuration files updated and queues synced successfully.")

if __name__ == "__main__":
    main()
