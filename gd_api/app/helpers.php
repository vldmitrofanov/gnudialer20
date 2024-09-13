<?php

use Illuminate\Support\Facades\File;
use Illuminate\Support\Facades\Storage;


if (!function_exists('convert_pjsip_to_text')) {
    function convert_pjsip_to_text($sip_config)
    {
        $ext = $sip_config['extension'];
        $auth_name = $sip_config['auth'];
        $password = $sip_config['password'];
        $context = $sip_config['context'];
        $callerid = $sip_config['callerid'];
        $max_contacts = $sip_config['max_contacts'];

        // Start with the endpoint configuration
        $result = "[{$ext}]\n";
        $result .= "type=endpoint\n";
        $result .= "context={$context}\n";
        $result .= "disallow={$sip_config['disallow']}\n";

        // Handle 'allow' variations
        foreach ($sip_config as $key => $value) {
            if (strpos($key, 'allow') === 0) {
                $result .= "allow={$value}\n";
            }
        }

        $result .= "auth={$auth_name}\n";
        $result .= "aors={$ext}\n";
        $result .= "callerid={$callerid}\n\n";

        // Add the auth section
        $result .= "[{$auth_name}]\n";
        $result .= "type=auth\n";
        $result .= "auth_type=userpass\n";
        $result .= "password={$password}\n";
        $result .= "username={$ext}\n\n";

        // Add the aor section
        $result .= "[{$ext}]\n";
        $result .= "type=aor\n";
        $result .= "max_contacts={$max_contacts}\n";

        return $result;
    }
}
if (!function_exists('createTzpopulateFile')) {
    function createTzpopulateFile($tableName)
    {
        // Define the directory and the file path
        $directory = storage_path('app/gnudialer/tzpopulate');
        $filePath = $directory . '/' . $tableName . '.tzpl';

        // Ensure the directory exists
        if (!File::exists($directory)) {
            File::makeDirectory($directory, 0755, true);
        }

        // Ensure there is no duplicate
        if (File::exists($filePath)) {
            return;
            //throw new \Exception('File for this table already exists.');
        }

        // Create the file
        File::put($filePath, ''); // Create an empty file
    }
}
