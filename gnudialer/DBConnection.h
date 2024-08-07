#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <memory>
#include <string>

class DBConnection {
public:
    DBConnection();
    ~DBConnection();

    std::shared_ptr<sql::Connection> getConnection();
    Campaign getCampaignByName(const std::string& name);
    std::vector<std::string> getCampaignSettings(u_long campaignId);

private:
    sql::mysql::MySQL_Driver* driver;
    std::shared_ptr<sql::Connection> conn;
    std::string host;
    std::string port;
    std::string user;
    std::string password;
    std::string database;

    void connect();
};

#endif // DBCONNECTION_H
