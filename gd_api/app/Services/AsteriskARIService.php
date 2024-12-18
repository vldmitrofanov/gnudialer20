<?php

namespace App\Services;

use Illuminate\Support\Facades\Cache;
use App\Models\Server;
use App\GnuDialer\Extra\ARICallPayload;
use App\GnuDialer\Extra\ARIRedirectPayload;
use Illuminate\Support\Facades\Log;

class AsteriskARIService
{
    protected $socket;
    protected $host = '127.0.0.1:8088';
    protected $proto = 'http';
    protected $username;
    protected $secret;
    protected $app = 'gnudialer_stasis_app';

    public function setServer($server_id)
    {
        $server = Cache::remember("server_ari_{$server_id}", 12 * 60 * 60, function () use ($server_id) {
            return Server::findOrFail($server_id)->data['ari'];
        });

        if (empty($server) || !isset($server['user'])) {
            throw new \Exception("Server configuration is incomplete or missing for server ID {$server_id}. 
            Please ensure that the server data contains the necessary AMI credentials, including 'user', 'host', 'port', and 'secret'.");
        }

        Log::info('ARI Server host: ' . $server['host']);
        // Assign the credentials
        $this->host = $server['host'];
        $this->proto = $server['proto'];
        $this->username = $server['user'];
        $this->secret = $server['password'];
    }

    public function getAllBridges()
    {
        // Construct the ARI URL for listing bridges
        $url = "{$this->proto}://{$this->host}/ari/bridges?api_key={$this->username}:{$this->secret}&app={$this->app}";

        // Initialize cURL session
        $ch = curl_init();

        // Set cURL options
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPGET, true);

        // Execute the request and fetch the response
        $response = curl_exec($ch);

        // Check if any error occurred during the request
        if (curl_errno($ch)) {
            $error_msg = curl_error($ch);
            Log::error('Error fetching ARI bridges: ' . $error_msg);
            throw new \Exception('Error fetching ARI bridges: ' . $error_msg);
        }

        // Close the cURL session
        curl_close($ch);

        // Parse the JSON response
        $bridges = json_decode($response, true);

        if (json_last_error() !== JSON_ERROR_NONE) {
            Log::error('Error parsing JSON response from ARI bridges');
            throw new \Exception('Error parsing JSON response from ARI bridges');
        }

        // Return the array of bridges
        return $bridges;
    }

    public function getBridgeById($bridge_id)
    {
        // Construct the ARI URL for listing bridges
        $url = "{$this->proto}://{$this->host}/ari/bridges/{$bridge_id}?api_key={$this->username}:{$this->secret}&app={$this->app}";

        // Initialize cURL session
        $ch = curl_init();

        // Set cURL options
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPGET, true);

        // Execute the request and fetch the response
        $response = curl_exec($ch);

        // Check if any error occurred during the request
        if (curl_errno($ch)) {
            $error_msg = curl_error($ch);
            Log::error('Error fetching ARI bridge: ' . $error_msg);
            throw new \Exception('Error fetching ARI bridge: ' . $error_msg);
        }

        // Close the cURL session
        curl_close($ch);

        // Parse the JSON response
        $bridge = json_decode($response, true);

        if (json_last_error() !== JSON_ERROR_NONE) {
            Log::error('Error parsing JSON response from ARI bridges');
            throw new \Exception('Error parsing JSON response from ARI bridges');
        }

        // Return the array of bridges
        return $bridge;
    }

    public function getAllChannels()
    {
        // Construct the ARI URL for listing bridges
        $url = "{$this->proto}://{$this->host}/ari/channels?api_key={$this->username}:{$this->secret}&app={$this->app}";

        // Initialize cURL session
        $ch = curl_init();

        // Set cURL options
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPGET, true);

        // Execute the request and fetch the response
        $response = curl_exec($ch);

        // Check if any error occurred during the request
        if (curl_errno($ch)) {
            $error_msg = curl_error($ch);
            Log::error('Error fetching ARI channels: ' . $error_msg);
            throw new \Exception('Error fetching ARI bridges: ' . $error_msg);
        }

        // Close the cURL session
        curl_close($ch);

        // Parse the JSON response
        $channels = json_decode($response, true);

        if (json_last_error() !== JSON_ERROR_NONE) {
            Log::error('Error parsing JSON response from ARI bridges');
            throw new \Exception('Error parsing JSON response from ARI bridges');
        }

        // Return the array of bridges
        return $channels;
    }

    public function getChannelById($channel_id)
    {
        // Construct the ARI URL for listing bridges
        $url = "{$this->proto}://{$this->host}/ari/channels/{$channel_id}?api_key={$this->username}:{$this->secret}&app={$this->app}";

        // Initialize cURL session
        $ch = curl_init();

        // Set cURL options
        curl_setopt($ch, CURLOPT_URL, $url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_HTTPGET, true);

        // Execute the request and fetch the response
        $response = curl_exec($ch);

        // Check if any error occurred during the request
        if (curl_errno($ch)) {
            $error_msg = curl_error($ch);
            Log::error('Error fetching ARI channel: ' . $error_msg);
            throw new \Exception('Error fetching ARI channel: ' . $error_msg);
        }

        // Close the cURL session
        curl_close($ch);

        // Parse the JSON response
        $channels = json_decode($response, true);

        if (json_last_error() !== JSON_ERROR_NONE) {
            Log::error('Error parsing JSON response from ARI bridges');
            throw new \Exception('Error parsing JSON response from ARI bridges');
        }

        // Return the array of bridges
        return $channels;
    }

    public function createChannel(ARICallPayload $payload)
    {
        $url = "{$this->proto}://{$this->host}/ari/channels?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $data = $payload->toArray();
        $ch = curl_init($url);

        // Set cURL options
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_POST, true);           // Use POST method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
        curl_setopt($ch, CURLOPT_POSTFIELDS,  json_encode($data));

        $response = curl_exec($ch);

        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }

        // Close cURL session
        curl_close($ch);

        // Return the response
        return $response;
    }

    public function redirectChannel(ARIRedirectPayload $payload)
    {
        $url = "{$this->proto}://{$this->host}/ari/channels/$payload->channel/redirect?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $data = $payload->toArray();
        $ch = curl_init($url);

        // Set cURL options
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_POST, true);           // Use POST method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
        curl_setopt($ch, CURLOPT_POSTFIELDS,  json_encode($data));

        $response = curl_exec($ch);

        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }

        // Close cURL session
        curl_close($ch);

        // Return the response
        return $response;
    }

    function putChannelOnHold($channelId)
    {
        $url = "{$this->proto}://{$this->host}/ari/channels/$channelId/hold?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $ch = curl_init($url);

        // Set cURL options
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_POST, true);           // Use POST method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);

        $response = curl_exec($ch);

        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }

        // Close cURL session
        curl_close($ch);
        $this->muteChannelInBridge($channelId);

        // Return the response
        return $response;
    }

    function muteChannelInBridge($channelId) {
        $url = "{$this->proto}://{$this->host}/ari/channels/$channelId/mute?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $ch = curl_init($url);
    
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_POST, true);           // Use POST method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
    
        $response = curl_exec($ch);
    
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }
    
        curl_close($ch);
        return $response;
    }

    function unmuteChannelInBridge($channelId) {
        $url = "{$this->proto}://{$this->host}/ari/channels/$channelId/unmute?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $ch = curl_init($url);
    
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_POST, true);           // Use POST method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
    
        $response = curl_exec($ch);
    
        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }
    
        // Close cURL session
        curl_close($ch);
    
        // Return the response
        return $response;
    }

    function releaseChannelFromHold($channelId)
    {
        $url = "{$this->proto}://{$this->host}/ari/channels/$channelId/hold?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $ch = curl_init($url);

        // Set cURL options
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE'); // Use DELETE method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);

        $response = curl_exec($ch);

        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }

        // Close cURL session
        curl_close($ch);
        $this->unmuteChannelInBridge($channelId);

        // Return the response
        return $response;
    }

    function channelHangup($channelId)
    {
        $url = "{$this->proto}://{$this->host}/ari/channels/{$channelId}?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $ch = curl_init($url);

        // Set cURL options
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE'); // Use DELETE method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);

        $response = curl_exec($ch);

        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }

        // Close cURL session
        curl_close($ch);

        // Return the response
        return $response;
    }

    function manualCall($data)
    {
        $url = "{$this->proto}://{$this->host}/ari/channels?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $ch = curl_init($url);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_POST, true);           // Use POST method
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);
        curl_setopt($ch, CURLOPT_POSTFIELDS,  json_encode($data));

        $response = curl_exec($ch);

        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            return 'Curl error: ' . $error;
        }

        // Close cURL session
        curl_close($ch);

        // Return the response
        return $response;
    }

    function findChannelByName($channelName)
    {
        $url = "{$this->proto}://{$this->host}/ari/channels?api_key={$this->username}:{$this->secret}&app={$this->app}";
        $ch = curl_init($url);

        // Set cURL options
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true); // Return the response
        curl_setopt($ch, CURLOPT_HTTPHEADER, ['Content-Type: application/json']);

        // Execute the GET request
        $response = curl_exec($ch);

        // Check for errors
        if ($response === false) {
            $error = curl_error($ch);
            curl_close($ch);
            throw new \Exception('Curl error: ' . $error);
        }

        // Close cURL session
        curl_close($ch);

        // Decode the response
        $channels = json_decode($response, true);

        if (json_last_error() !== JSON_ERROR_NONE) {
            throw new \Exception('Failed to decode JSON response: ' . json_last_error_msg());
        }

        // Search for the channel by name
        foreach ($channels as $channel) {
            if (isset($channel['name']) && $channel['name'] === $channelName) {
                return $channel; // Return the found channel object
            }
        }

        // If no channel is found, return null
        return null;
    }
}
