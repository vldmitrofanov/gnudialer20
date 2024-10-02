#!/bin/bash

# Path to Laravel .env file
ENV_FILE="../gd_api/.env"

# Function to read values from the .env file
function get_env_value() {
    grep -oP "(?<=^$1=).*" "$ENV_FILE" | tr -d '"'
}

# Fetch MySQL credentials from the .env file
DB_HOST=$(get_env_value "DB_HOST")
DB_USER=$(get_env_value "DB_USERNAME")
DB_PASS=$(get_env_value "DB_PASSWORD")
DB_NAME=$(get_env_value "DB_DATABASE")
DB_PORT=$(get_env_value "DB_PORT")

# Table and column name
TABLE_NAME="users"
COLUMN_NAME="ip_address"

# Fetch IP addresses from MySQL
IP_ADDRESSES=$(mysql -h $DB_HOST -P $DB_PORT -u $DB_USER -p$DB_PASS -D $DB_NAME -se "SELECT DISTINCT $COLUMN_NAME FROM $TABLE_NAME WHERE NOT NULL $COLUMN_NAME;")

# Check if there are any IP addresses returned
if [ -z "$IP_ADDRESSES" ]; then
    echo "No IP addresses found in the database."
    exit 1
fi

function is_ip_allowed() {
    local ip=$1
    iptables -L INPUT -v -n | grep -q "$ip"
    return $?
}

# Loop through each IP and allow it through the firewall
echo "Applying firewall rules for the following IPs:"
for ip in $IP_ADDRESSES; do
    echo "Processing IP: $ip"

    if ! is_ip_allowed "$ip"; then
        echo "IP $ip is not allowed, adding rules."
        # Open SIP (UDP 5060)
        iptables -A INPUT -s "$ip" -p udp --dport 5060 -j ACCEPT

        # Open media port range (UDP 10000-20000)
        iptables -A INPUT -s "$ip" -p udp --dport 10000:20000 -j ACCEPT

        # Open port 8443 (TCP)
        iptables -A INPUT -s "$ip" -p tcp --dport 8443 -j ACCEPT
    else
        echo "IP $ip is already allowed, skipping."
    fi
done

# Save iptables rules to persist them
#iptables-save > /etc/iptables/rules.v4

echo "Firewall rules successfully applied."
