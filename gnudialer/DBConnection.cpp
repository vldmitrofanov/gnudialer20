#include "DBConnection.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <vector>       
#include <stdexcept>
#include "Campaign.h"
#include "etcinfo.h"
#include "ParsedAgent.h"

DBConnection::DBConnection() {
    driver = sql::mysql::get_mysql_driver_instance();
    host = getMySqlHost().c_str();
    port = getMysqlPort().c_str();
    user = getMySqlUser().c_str();
    password = getMySqlPass().c_str();
    database = getDbName().c_str();
    connect();
}

DBConnection::~DBConnection() {
    if (conn) {
        conn->close();
    }
}

void DBConnection::connect() {
    try {
        std::string connectionString = "tcp://" + host + ":" + port;
        conn = std::shared_ptr<sql::Connection>(driver->connect(connectionString, user, password));
        conn->setSchema(database);
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
}

std::shared_ptr<sql::Connection> DBConnection::getConnection() {
    if (!conn || conn->isClosed()) {
        connect();
    }
    return conn;
}

std::vector<std::string> DBConnection::getCampaigns(){
    std::vector<std::string> campaigns;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT code FROM campaigns WHERE status = 1"));
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            std::string parameter = res->getString("code");        
            campaigns.push_back(parameter);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return campaigns;
}

Campaign DBConnection::getCampaignByName(const std::string& name) {
    Campaign campaign;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT id, name FROM campaigns WHERE code = ? AND status > 0"));
        pstmt->setString(1, name);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            campaign.id = res->getUInt64("id");
            campaign.name = res->getString("name");
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return campaign;
}

std::vector<std::string> DBConnection::getCampaignSettings(u_long campaignId, u_long serverId) {
    std::vector<std::string> itsSettings;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement(
                "SELECT settings.parameter as parameter, settings.value as value FROM settings LEFT JOIN queues ON settings.queue_id = queues.id  WHERE queues.campaign_id = ? AND queues.server_id = ?"));
        pstmt->setUInt64(1, campaignId);
        pstmt->setUInt64(2, serverId);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            std::string parameter = res->getString("parameter");
            std::string value = res->getString("value");
            std::string tempLine = parameter + ":" + value;
            itsSettings.push_back(tempLine);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsSettings;
}

std::vector<std::string> DBConnection::getCampaignFilters(u_long campaignId,u_long serverId) {
    std::vector<std::string> itsFilters;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT filters.filter as filter FROM filters LEFT JOIN queues ON filters.queue_id = queues.id  WHERE queues.campaign_id = ? AND queues.server_id = ?"));
        pstmt->setUInt64(1, campaignId);
        pstmt->setUInt64(2, serverId);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            std::string parameter = res->getString("filter");
            itsFilters.push_back(parameter);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsFilters;
}

std::vector<u_long> DBConnection::getCampaignAgents(u_long campaignId, u_long serverId) {
    std::vector<u_long> itsAgents;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT agent_queue.agent_id as agent_id FROM agent_queue LEFT JOIN queues ON agent_queue.queue_id = queues.id  WHERE queues.campaign_id = ? AND queues.server_id = ?"));
        pstmt->setUInt64(1, campaignId);
        pstmt->setUInt64(2, serverId);
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            std::string parameter = res->getString("agent_id");
            itsAgents.push_back(std::stoi(parameter));
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsAgents;
}

std::vector<ParsedAgent> DBConnection::getAllAgents(u_long serverId) {
   std::vector<ParsedAgent> itsAgents;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT agent_queue.agent_id AS agent_id, agents.password AS password, CONCAT(users.first_name, ' ', users.last_name) AS name FROM agent_queue LEFT JOIN queues ON agent_queue.queue_id = queues.id LEFT JOIN agents ON agent_queue.agent_id = agents.id LEFT JOIN users ON agents.user_id = users.id WHERE queues.server_id = ?"));
        pstmt->setUInt64(1, serverId);  // Corrected parameter index
        std::shared_ptr<sql::ResultSet> res(pstmt->executeQuery());

        while (res->next()) {
            ParsedAgent agent;
            agent.herNumber = res->getInt("agent_id");
            agent.herName = res->getString("name");
            agent.herPass = res->getString("password");
            itsAgents.push_back(agent);
        }
    } catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "MySQL error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQLState: " << e.getSQLState() << std::endl;
    }
    return itsAgents;
}
