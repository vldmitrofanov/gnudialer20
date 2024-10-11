#include <iostream>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include "etcinfo.h"
#include "DBConnection.h"  
#include "HttpClient.h"
// THIS IS NOT YET READY
using json = nlohmann::json;

struct ChannelData {
    std::string leadId;
    std::string campaign;
    std::string dspMode;
    std::string isTransfer;
};

// Callback function for CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to handle the ARI Stasis event
void handleStasisStart(const json& eventData) {
    try {
        ChannelData data;
        // Extract variables from the ARI event JSON
        data.leadId = eventData["channel"]["variables"]["LEADID"];
        data.campaign = eventData["channel"]["variables"]["CAMPAIGN"];
        data.dspMode = eventData["channel"]["variables"]["DSPMODE"];
        data.isTransfer = eventData["channel"]["variables"]["ISTRANSFER"];
        std::string channelId = eventData["channel"]["id"];

        std::cout << "Received StasisStart event!" << std::endl;
        std::cout << "Lead ID: " << data.leadId << std::endl;
        std::cout << "Campaign: " << data.campaign << std::endl;
        std::cout << "DSP Mode: " << data.dspMode << std::endl;
        std::cout << "Is Transfer: " << data.isTransfer << std::endl;

        // Use DBConnection to query or log something in the database (if needed)
        DBConnection dbConn;
        u_long serverId = std::stoull(getServerId());
        //dbConn.logEvent(data.leadId, data.campaign, serverId);  // Assuming logEvent is a custom function you have

        // Perform other actions based on the extracted data

    } catch (const std::exception& ex) {
        std::cerr << "Error parsing Stasis event: " << ex.what() << std::endl;
    }
}

// Function to subscribe to ARI events
void subscribeToAriEvents(const std::string& ariHost, const std::string& ariUser, const std::string& ariPass) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://" + ariHost + ":8088/ari/events?app=my_stasis_app&api_key=" + ariUser + ":" + ariPass;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform request and handle the ARI events
        while (true) {
            res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                break;
            }

            try {
                // Parse the JSON response from ARI
                json eventData = json::parse(readBuffer);

                // Check if the event is of type 'StasisStart'
                if (eventData["type"] == "StasisStart") {
                    handleStasisStart(eventData);
                }

                // Clear buffer for next event
                readBuffer.clear();

            } catch (const std::exception& ex) {
                std::cerr << "Error parsing JSON response: " << ex.what() << std::endl;
            }
        }

        curl_easy_cleanup(curl);
    }
}

bool isCallAnswered(const std::string& channelId) {
    try {
        HttpClient client(getMainHost(), 8088, getAriUser(), getAriPass());
        
        // Build the URL to query the channel's state
        std::string channelStateUrl = "/ari/channels/" + channelId + "?api_key=" + getAriUser() + ":" + getAriPass();
        
        // Get the response from ARI
        std::string response = client.get(channelStateUrl);
        json channelData = json::parse(response);

        // Log the response for debugging
        std::cout << "[DEBUG] Channel state response: " << response << std::endl;

        // Check if the channel is in the 'Up' state (indicating the call is answered)
        if (channelData.contains("state") && channelData["state"] == "Up") {
            std::cout << "[DEBUG] Channel " << channelId << " is answered (state is Up)." << std::endl;
            return true;
        } else {
            std::cout << "[DEBUG] Channel " << channelId << " is not answered (state is " << channelData["state"] << ")." << std::endl;
            return false;
        }
    } catch (const std::exception &ex) {
        std::cerr << "Error checking channel state for " << channelId << ": " << ex.what() << std::endl;
        return false;
    }
}

int main() {
    std::string ariHost = getMainHost();  // Fetch from your configuration
    std::string ariUser = getAriUser();   // Fetch ARI username from configuration
    std::string ariPass = getAriPass();   // Fetch ARI password from configuration

    std::cout << "Starting ARI Stasis App..." << std::endl;

    // Subscribe to ARI events
    subscribeToAriEvents(ariHost, ariUser, ariPass);

    return 0;
}
