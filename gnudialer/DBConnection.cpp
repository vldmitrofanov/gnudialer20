#include "DBConnection.h"
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include "Campaign.h"

// Example configuration parsing functions
std::string getMySqlHost() { return "localhost"; }
std::string getMysqlPort() { return "3306"; }
std::string getMySqlUser() { return "root"; }
std::string getMySqlPass() { return "password"; }
std::string getMysqlDbName() { return "mydatabase"; }

DBConnection::DBConnection() {
    driver = sql::mysql::get_mysql_driver_instance();
    host = getMySqlHost();
    port = getMysqlPort();
    user = getMySqlUser();
    password = getMySqlPass();
    database = getMysqlDbName();
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

Campaign DBConnection::getCampaignByName(const std::string& name) {
    Campaign campaign;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT id, name FROM campaigns WHERE name = ? AND status > 0"));
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

std::vector<std::string> DBConnection::getCampaignSettings(u_long campaignId) {
    std::vector<std::string> itsSettings;
    try {
        std::shared_ptr<sql::PreparedStatement> pstmt(
            conn->prepareStatement("SELECT parameter, value FROM campaign_settings WHERE campaign_id = ?"));
        pstmt->setUInt64(1, campaignId);
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
