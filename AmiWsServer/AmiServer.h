#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <vector>
#include <string>
#include "etcinfo.h"

struct AmiServer {
    u_long id;
    std::string host;
    std::string username;
    std::string password;
};

std::vector<AmiServer> getAmiServers() {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
    std::string mysqlHost = getMySqlHost();
    std::string mysqlPort = getMysqlPort();
    std::string mysqlUser = getMySqlUser();
    std::string mysqlPass = getMySqlPass();
    std::string mysqlDbName = getMysqlDbName();
    sql::Connection* con = driver->connect("tcp://"+mysqlHost+":"+mysqlPort, mysqlUser, mysqlPass);
    con->setSchema(mysqlDbName);

    std::vector<AmiServer> servers;
    sql::PreparedStatement* pstmt = con->prepareStatement("SELECT id, host, username, secret FROM servers");
    sql::ResultSet* res = pstmt->executeQuery();

    while (res->next()) {
        AmiServer server;
        server.id = res->getUInt64("id");
        server.host = res->getString("host");
        server.username = res->getString("username");
        server.password = res->getString("secret");
        servers.push_back(server);
    }

    delete res;
    delete pstmt;
    delete con;

    return servers;
}
