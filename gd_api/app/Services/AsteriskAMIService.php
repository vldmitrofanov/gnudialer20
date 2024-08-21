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

    public function sendCommand($command, $end)
    {
        fwrite($this->socket, $command);

        $response = '';
        $startTime = time(); // Track the start time
        $timeout = 10; // Set a timeout in seconds

        while (!feof($this->socket)) {
            $line = fgets($this->socket);
            if ($line === false) {
                break;
            }
    
            $response .= $line;
    
            // Log each line for debugging purposes
            Log::info("AMI Response Line: {$line}");
    
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
    
        return $response;
    }

    public function getAgentStatus($queue, $agent)
    {
        // Prepare the QueueStatus command
        $command = "Action: QueueStatus\r\n";
        $command .= "Queue: $queue\r\n\r\n";

        // Send the command to AMI
        $response = $this->sendCommand($command, "Event: QueueStatusComplete");
        Log::info("AMI Response: {$response}");
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
        $lines = explode("\n", $response);
        $agentStatus = [];
        $inAgentBlock = false;
    
        foreach ($lines as $line) {
            $line = trim($line);  // Remove any extra whitespace
    
            // Look for the start of the agent's block
            if (strpos($line, "Name: PJSIP/{$agent}") !== false) {
                $inAgentBlock = true;
            }
    
            // If we're in the agent's block, capture relevant information
            if ($inAgentBlock) {
                if (strpos($line, 'Paused:') !== false) {
                    $agentStatus['Paused'] = trim(explode(':', $line)[1]);
                }
                if (strpos($line, 'Status:') !== false) {
                    $agentStatus['Status'] = trim(explode(':', $line)[1]);
                }
    
                // Add other fields as needed
                if (strpos($line, 'InCall:') !== false) {
                    $agentStatus['InCall'] = trim(explode(':', $line)[1]);
                }
    
                // Exit the block after processing all relevant information
                if (empty($line)) {
                    break;
                }
            }
        }
    
        if (empty($agentStatus)) {
            return "Agent not found in the queue or no status available.";
        }
    
        return $agentStatus;
    }

    public function sendCustomEvent($eventName, $variables = [])
    {
        $command = "Action: UserEvent\r\n";
        $command .= "UserEvent: {$eventName}\r\n";

        foreach ($variables as $key => $value) {
            $command .= "{$key}: {$value}\r\n";
        }

        $command .= "\r\n";

        return $this->sendCommand($command);
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

    public function __destruct()
    {
        $this->logout();
    }
}
