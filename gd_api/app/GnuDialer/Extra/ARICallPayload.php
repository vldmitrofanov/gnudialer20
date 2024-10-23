<?php
namespace App\GnuDialer\Extra;
class ARICallPayload
{
    public $endpoint;    // SIP or PJSIP endpoint (e.g., PJSIP/1000)
    public $extension;   // Extension to dial (e.g., 100)
    public $context;     // Dialplan context (e.g., from-internal)
    public $priority;    // Dialplan priority (e.g., 1)
    public $variables;

    public function __construct($endpoint, $extension, $context, $priority, $variables)
    {
        $this->endpoint = $endpoint;
        $this->extension = $extension;
        $this->context = $context;
        $this->priority = $priority;
        $this->variables = $variables;
    }

    public function toArray()
    {
        return [
            'endpoint' => $this->endpoint,
            'extension' => $this->extension,
            'context' => $this->context,
            'priority' => $this->priority,
            'variables' => $this->variables,
        ];
    }
}