#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mutex>
#include <nlohmann/json.hpp>
#include "Socket.h"
#include "etcinfo.h"
#include "AmiServer.h"
#define WS_PORT 9002

typedef websocketpp::server<websocketpp::config::asio> wsServer;
using json = nlohmann::json;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

// pull out the type of messages sent by our config
typedef wsServer::message_ptr message_ptr;

std::unordered_map<std::string, std::string> token_cache;

std::map<u_long, ClientSocket*> amiSockets; // Map to store ClientSocket instances indexed by server ID
std::mutex admin_hdl_mutex;
websocketpp::connection_hdl admin_hdl;
wsServer server;

sql::Connection *connectToDatabase()
{
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    std::string mysqlHost = getMySqlHost();
    std::string mysqlPort = getMysqlPort();
    std::string mysqlUser = getMySqlUser();
    std::string mysqlPass = getMySqlPass();
    std::string mysqlDbName = getMysqlDbName();
    sql::Connection *con = driver->connect("tcp://" + mysqlHost + ":" + mysqlPort, mysqlUser, mysqlPass);
    con->setSchema(mysqlDbName);
    return con;
}

bool validateToken(const std::string &token)
{
    auto it = token_cache.find(token);
    if (it != token_cache.end())
    {
        return true;
    }

    sql::Connection *con = connectToDatabase();
    sql::PreparedStatement *pstmt = con->prepareStatement("SELECT COUNT(*) FROM personal_access_tokens WHERE token = ?");
    pstmt->setString(1, token);
    sql::ResultSet *res = pstmt->executeQuery();

    bool valid = false;
    if (res->next() && res->getInt(1) > 0)
    {
        valid = true;
        token_cache[token] = token;
    }

    delete res;
    delete pstmt;
    delete con;

    return valid;
}
void handle_action(const std::string& action, const std::string& param, ClientSocket* amiSocket) 
{
    std::string response;
    if (action == "Logoff" || action == "AgentLogoff")
    {
        (*amiSocket) << "Action: Agentlogoff\r\n";
        (*amiSocket) << "Agent: " << param << "\r\n\r\n";
    }
    else if (action == "AgentPause")
    {
        (*amiSocket) << "Action: AgentPause\r\n";
        (*amiSocket) << "Agent: " << param << "\r\n";
        (*amiSocket) << "Pause: true\r\n\r\n"; // Assuming `tf` is true for the example
    }
    else if (action == "MUE_AgentPause")
    {
        (*amiSocket) << "Action: sendevent\r\n";
        (*amiSocket) << "Event: AgentPause\r\n";
        (*amiSocket) << "Body: Agent: " << param << "\r\n\r\n";
    }
    else if (action == "MUE_AgentUnPause")
    {
        (*amiSocket) << "Action: sendevent\r\n";
        (*amiSocket) << "Event: AgentUnPause\r\n";
        (*amiSocket) << "Body: Agent: " << param << "\r\n\r\n";
    }
    else if (action == "CliCommand")
    {
        (*amiSocket) << "Action: Command\r\n";
        (*amiSocket) << "Command: "<< param <<"\r\n\r\n";
        (*amiSocket) >> response;
        std::cout << "Core reload response: " << response << std::endl;
    }
}
void onMessage(wsServer *s, websocketpp::connection_hdl hdl, message_ptr msg)
{
    try
    {
        json received_json = json::parse(msg->get_payload());
        /*
        if (!validateToken(token))
        {
            s->close(hdl, websocketpp::close::status::going_away, "Invalid Token");
            return;
        }
        */

        if (received_json.contains("action") && received_json.contains("param") && received_json.contains("server_id"))
        {
            std::string action = received_json["action"];
            std::string param = received_json["param"].get<std::string>();
            u_long server_id = received_json["server_id"].get<u_long>();

            // Find the corresponding AMI socket
            auto it = amiSockets.find(server_id);
            if (it != amiSockets.end())
            {
                // Handle the action
                handle_action(action, param, it->second);

                // Example of sending a response back to the client
                json response;
                response["status"] = "success";
                response["action"] = action;
                s->send(hdl, response.dump(), websocketpp::frame::opcode::text);
            }
            else
            {
                std::cerr << "Invalid server ID: " << server_id << std::endl;
                s->send(hdl, "Invalid server ID", websocketpp::frame::opcode::text);
            }
        }
        else
        {
            std::cerr << "Invalid JSON format: Missing action, param, or server_id" << std::endl;
            s->send(hdl, "Invalid JSON format: Missing action, param, or server_id", websocketpp::frame::opcode::text);
        }
    }
    catch (json::parse_error &e)
    {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        s->send(hdl, "Invalid JSON format", websocketpp::frame::opcode::text);
    }
    catch (websocketpp::exception const &e)
    {
        std::cerr << "WebSocket++ exception: " << e.what() << std::endl;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
    }
    // Token is valid, handle the message
    // Example: send a response back to the client
    // s->send(hdl, "Authenticated", websocketpp::frame::opcode::text);
}

void listenToAMI(const AmiServer &amiServer, int write_fd, wsServer *wsServer, websocketpp::connection_hdl admin_hdl)
{
    std::string response, block;
    ClientSocket amiSocket(amiServer.host, 5038);
    try
    {
        amiSocket >> response;
        amiSocket << "Action: Login\r\nUserName: " + amiServer.username + "\r\nSecret: " + amiServer.password + "\r\nEvents:off\r\n\r\n";
        amiSocket >> response;

        // Check if login was successful
        if (response.find("Success") == std::string::npos)
        {
            std::cerr << "AMI login failed for " << amiServer.host << std::endl;
            return;
        }

        // Simulated AMI response for testing
        std::string amiResponse = "Event: AgentLogin\nAgent: 1001\n\n";

        std::stringstream BlockStream;
        BlockStream.str(amiResponse);

        for (std::string tempLine; std::getline(BlockStream, tempLine, '\n');)
        {
            if (!tempLine.empty() && tempLine.back() == '\r')
            {
                tempLine.pop_back();
            }

            if (tempLine.empty())
            {
                if (!block.empty())
                {
                    std::cout << "Processing block:\n"
                              << block << std::endl;

                    // Send event to admin channel
                    if (auto lock = admin_hdl.lock())
                    {
                        try
                        {
                            wsServer->send(lock, block, websocketpp::frame::opcode::text);
                        }
                        catch (const websocketpp::exception &e)
                        {
                            std::cerr << "Error sending message: " << e.what() << std::endl;
                        }
                    }
                    else
                    {
                        std::cerr << "Admin connection handle is invalid." << std::endl;
                    }

                    if (block.find("Event: AgentLogin") != std::string::npos)
                    {
                        size_t pos = block.find("Agent: ");
                        std::string agentId = block.substr(pos + 7);
                        agentId.erase(agentId.find_first_of('\n'));

                        std::string agentChannel = "agent." + agentId;
                        std::cout << "Sending to agent channel: " << agentChannel << std::endl;

                        // Send event to agent channel
                        // Note: In a real scenario, you would need to maintain a mapping of agent channels to connection handles
                    }

                    block.clear();
                }
            }
            else
            {
                block += tempLine + '\n';
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in listenToAMI: " << e.what() << std::endl;
    }
}

int main()
{
    std::vector<AmiServer> servers = getAmiServers();
    int numServers = servers.size();
    int pipes[numServers][2];

    wsServer wsServer;
    websocketpp::connection_hdl admin_hdl;
    std::mutex admin_hdl_mutex;
    try
    {
        // Set logging settings
        wsServer.set_access_channels(websocketpp::log::alevel::all);
        wsServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize Asio
        wsServer.init_asio();

        wsServer.set_open_handler([&admin_hdl, &admin_hdl_mutex](websocketpp::connection_hdl hdl)
                                  {
        std::lock_guard<std::mutex> lock(admin_hdl_mutex);
        admin_hdl = hdl;
        std::cout << "Connection opened. Handle: " << hdl.lock().get() << std::endl; });

        wsServer.set_close_handler([&admin_hdl, &admin_hdl_mutex](websocketpp::connection_hdl hdl)
                                   {
        std::lock_guard<std::mutex> lock(admin_hdl_mutex);
        if (hdl.lock() == admin_hdl.lock()) {
            admin_hdl.reset();
            std::cout << "Admin connection closed. Handle: " << hdl.lock().get() << std::endl;
        } else {
            std::cout << "Connection closed. Handle: " << hdl.lock().get() << std::endl;
        } });

        wsServer.set_fail_handler([&admin_hdl, &admin_hdl_mutex](websocketpp::connection_hdl hdl)
                                  {
        std::lock_guard<std::mutex> lock(admin_hdl_mutex);
        if (hdl.lock() == admin_hdl.lock()) {
            admin_hdl.reset();
            std::cerr << "Admin connection failed. Handle: " << hdl.lock().get() << std::endl;
        } else {
            std::cerr << "Connection failed. Handle: " << hdl.lock().get() << std::endl;
        } });

        wsServer.set_message_handler(bind(&onMessage, &wsServer, ::_1, ::_2));

        // Correctly specify the endpoint
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), WS_PORT);
        wsServer.listen(endpoint);
        wsServer.start_accept();

        // Run the WebSocket server in a separate thread
        std::thread ws_thread([&wsServer]()
                              { wsServer.run(); });

        // Start threads to listen to AMI events
        for (int i = 0; i < numServers; ++i)
        {
            AmiServer &server = servers[i];
            amiSockets[server.id] = new ClientSocket(server.host, 5038);

            if (pipe(pipes[i]) == -1)
            {
                std::cerr << "Failed to create pipe: " << strerror(errno) << std::endl;
                return 1;
            }

            pid_t pid = fork();
            if (pid == -1)
            {
                std::cerr << "Failed to fork: " << strerror(errno) << std::endl;
                return 1;
            }
            else if (pid == 0)
            {
                // Child process
                close(pipes[i][0]); // Close read end
                listenToAMI(server, pipes[i][1], &wsServer, admin_hdl);
                close(pipes[i][1]);
                exit(0);
            }
            else
            {
                // Parent process
                close(pipes[i][1]); // Close write end
            }
        }

        // Wait for the WebSocket server thread to finish
        ws_thread.join();
    }
    catch (websocketpp::exception const &e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "other exception" << std::endl;
    }

    return 0;
}
