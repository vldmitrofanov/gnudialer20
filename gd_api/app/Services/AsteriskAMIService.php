<?php

namespace App\Services;

use Illuminate\Support\Facades\Cache;
use App\Models\Server;
use Illuminate\Support\Facades\Log;

class AsteriskAMIService
{
    protected $socket;
    protected $host = '127.0.0.1';
    protected $port = 5038;
    protected $username;
    protected $secret;

    public function setServer($server_id)
    {
        $server = Cache::remember("server_ami_{$server_id}", 12 * 60 * 60, function () use ($server_id) {
            return Server::findOrFail($server_id)->data['ami'];
        });

        if (empty($server) || !isset($server['username'])) {
            throw new \Exception("Server configuration is incomplete or missing for server ID {$server_id}. 
            Please ensure that the server data contains the necessary AMI credentials, including 'username', 'host', 'port', and 'secret'.");
        }

        Log::info('AMI Server host: ' . $server['host']);
        // Assign the credentials
        $this->host = $server['host'];
        $this->port = $server['port'];
        $this->username = $server['username'];
        $this->secret = $server['secret'];

        // Connect to AMI
        $this->connect();
    }

    protected function connect()
    {
        $this->socket = fsockopen($this->host, $this->port, $errno, $errstr, 30);

        if (!$this->socket) {
            throw new \Exception("Could not connect to Asterisk AMI: $errstr ($errno)");
        }

        $this->login();
    }

    protected function login()
    {
        $login = "Action: Login\r\n";
        $login .= "Username: {$this->username}\r\n";
        $login .= "Secret: {$this->secret}\r\n\r\n";

        fwrite($this->socket, $login);
        $response = '';
        $authenticated = false;
        $maxAttempts = 10; // Limit the number of iterations to avoid an infinite loop

        // Read the response in chunks
        for ($i = 0; $i < $maxAttempts; $i++) {
            $line = fgets($this->socket, 4096);
            if ($line === false) {
                break; // Stop if there is no more data
            }

            $response .= $line;
            // Log::info($line);

            // Check if the response contains 'Authentication accepted'
            if (strpos($line, 'Authentication accepted') !== false) {
                $authenticated = true;
                break;
            }

            // If the response indicates the end of a response block, stop reading
            if (trim($line) === '') {
                break;
            }
        }

        // Log::info('Authentication successful');
        // If 'Authentication accepted' was not found, throw an exception
        if (!$authenticated) {
            throw new \Exception("Failed to authenticate with Asterisk AMI. Response: " . $response);
        }
    }

    public function sendCommand($command, $end, $noReturn = false)
    {
        Log::info("AMI Command Line: {$command}");
        fwrite($this->socket, $command);
        $ends = explode("|",$end);

        $response = '';
        $startTime = time(); // Track the start time
        $timeout = 30; // Set a timeout in seconds
        $responseLog = "";
        if($noReturn) {
            return "Ok";
        }
        while (!feof($this->socket)) {
            $line = fgets($this->socket);
            if ($line === false) {
                break;
            }
            $response .= $line;
            $responseLog .= $line;
            // Break if the end of the QueueStatus response is reached
            foreach($ends as $e1){
                if (strpos($line, $e1) !== false) {
                    break;
                }
            }

            // Implement a timeout to avoid infinite loops
            if ((time() - $startTime) > $timeout) {
                Log::warning("AMI response timed out after {$timeout} seconds");
                break;
            }
        }
        Log::info("AMI Response: {$responseLog}");

        return $this->parseResponse($response);
    }

    protected function parseResponse($response)
    {
        $parsedResponse = [];
        $lines = explode("\n", $response); // Split response into lines
        $currentBlock = []; // To hold the current block of key-value pairs
        $blockIdentifier = null; // To identify the start of a new block

        foreach ($lines as $line) {
            $line = trim($line); // Trim any whitespace

            if (empty($line)) {
                // If the line is empty, it might indicate the end of a block
                if (!empty($currentBlock)) {
                    if ($blockIdentifier) {
                        // Store the current block using the block identifier
                        $parsedResponse[$blockIdentifier][] = $currentBlock;
                    } else {
                        // If there's no block identifier, just append the block
                        $parsedResponse[] = $currentBlock;
                    }
                    $currentBlock = []; // Reset for the next block
                }
                continue;
            }

            if (strpos($line, ':') !== false) {
                list($key, $value) = explode(':', $line, 2);
                $key = trim($key);
                $value = trim($value);

                if (!empty($key)) {
                    if ($key === 'Event') {
                        // Assume 'Event' indicates the start of a new block
                        $blockIdentifier = $value;
                    }
                    $currentBlock[$key] = $value;
                }
            }
        }

        // Add the last block if it's not empty
        if (!empty($currentBlock)) {
            if ($blockIdentifier) {
                $parsedResponse[$blockIdentifier][] = $currentBlock;
            } else {
                $parsedResponse[] = $currentBlock;
            }
        }

        return $parsedResponse;
    }


    public function getAgentStatus($queue, $agent)
    {
        // Prepare the QueueStatus command
        $command = "Action: QueueStatus\r\n";
        $command .= "Queue: $queue\r\n\r\n";

        // Send the command to AMI
        $response = $this->sendCommand($command, "Event: QueueStatusComplete");
        Log::info("AMI Response: " . serialize($response));
        // Parse the response to find the specific agent's status
        $agentStatus = $this->parseQueueStatusResponse($response, $agent);

        return $agentStatus;
    }

    public function setAgentPause($queue, $agent, $pause)
    {
        $command = "Action: QueuePause\r\n";
        $command .= "Queue: $queue\r\n";
        $command .= "Interface: PJSIP/{$agent}\r\n";
        $command .= "Paused: " . ($pause ? "true" : "false") . "\r\n\r\n";

        return $this->sendCommand($command, "PausedReason:");
    }

    protected function parseQueueStatusResponse($response, $agent)
    {
        $agentStatus = [];
        $isFlat = false;
        foreach ($response as $key => $val) {
            if (is_array($val)) {
                if ($key == 'QueueMember') {
                    foreach ($val as $block) {
                        $this->parseAgentStatusBlock($agentStatus, $block, $agent);
                    }
                }
            } else {
                $isFlat = true;
            }
        }
        if ($isFlat) {
            $this->parseAgentStatusBlock($agentStatus, $response, $agent);
        }

        if (empty($agentStatus)) {
            return "Agent not found in the queue or no status available.";
        }

        return $agentStatus;
    }

    private function parseAgentStatusBlock(&$agentStatus, $block, $agent)
    {

        if ($block['Name'] == "PJSIP/{$agent}") {
            $agentStatus['Paused'] = $block['Paused'];
            $agentStatus['Status'] = $block['Status'];
            $agentStatus['InCall'] = $block['InCall'];
        }
    }

    public function sendCustomEvent($eventName, $variables = [], $end = "\r\n\r\n")
    {
        $command = "Action: UserEvent\r\n";
        $command .= "UserEvent: {$eventName}\r\n";

        foreach ($variables as $key => $value) {
            $command .= "{$key}: {$value}\r\n";
        }

        $command .= "\r\n";

        return $this->sendCommand($command, $end);
    }

    public function channelHangup($channel)
    {
        $command = "Action: Hangup\r\n";
        $command .= "Channel: {$channel}\r\n\r\n";
        return $this->sendCommand($command, "\r\n");
    }

    public function logout()
    {
        if ($this->socket) {
            // Logoff from AMI
            $logoff = "Action: Logoff\r\n\r\n";
            fwrite($this->socket, $logoff);

            // Close the socket
            fclose($this->socket);
            $this->socket = null; // Ensure the socket is not reused
        }
    }

    public function sendCommandAndGetChannell($command, $end)
    {
        Log::info("AMI Command Line: {$command}");
        fwrite($this->socket, $command);

        $response = '';
        $startTime = time(); // Track the start time
        $timeout = 10; // Set a timeout in seconds
        $channel = ''; // Variable to store the channel name

        while (!feof($this->socket)) {
            $line = fgets($this->socket);
            if ($line === false) {
                break;
            }

            $response .= $line;

            // Log each line for debugging purposes
            Log::info("AMI Response Line: {$line}");

            // Capture the channel name from the response
            if (strpos($line, 'Channel:') !== false) {
                $channel = trim(explode(":", $line)[1]); // Extract and trim the channel name
                Log::info("Captured Channel Name: {$channel}");
                break;
            }

            // Check for "Newstate" event indicating the call was answered
            if (strpos($line, 'Event: Newstate') !== false && strpos($line, 'ChannelStateDesc: Up') !== false) {
                Log::info("The third party has answered the call on Channel: {$channel}");
                // You can add more logic here to handle the event when the call is picked up
            }

            // Break if the end of the QueueStatus response is reached
            if (strpos($line, $end) !== false) {
                break;
            }

            // Implement a timeout to avoid infinite loops
            if ((time() - $startTime) > $timeout) {
                Log::warning("AMI response timed out after {$timeout} seconds");
                break;
            }
        }

        // Optionally return the response or the channel name as needed
        return ['response' => $this->parseResponse($response), 'channel' => $channel];
    }

    public function joinBridge($bridgeId, $channel)
    {
        $command = "Action: Bridge\r\n";
        $command .= "BridgeUniqueid: {$bridgeId}\r\n";
        $command .= "Channel: {$channel}\r\n";  // The third party's SIP channel
        $command .= "ActionID: addThirdPartyToBridge\r\n\r\n";

        return $this->sendCommand($command, "\r\n");
    }
    

    public function __destruct()
    {
        $this->logout();
    }
}
