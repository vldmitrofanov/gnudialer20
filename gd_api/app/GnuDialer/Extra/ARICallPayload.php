<?php
namespace App\GnuDialer\Extra;
class ARICallPayload
{
    public $endpoint;    // SIP or PJSIP endpoint (e.g., PJSIP/1000)
    public $extension;   // Extension to dial (e.g., 100)
    public $context;     // Dialplan context (e.g., from-internal)
    public $priority;    // Dialplan priority (e.g., 1)
    public $variables;
    public $callerId;

    public function __construct($endpoint, $extension, $context, $priority, $callerId, $variables)
    {
        $this->endpoint = $endpoint;
        $this->extension = $extension;
        $this->context = $context;
        $this->priority = $priority;
        $this->callerId = $callerId;
        $this->variables = array_map('strval', $variables);
    }

    public function toArray()
    {
        return [
            'endpoint' => $this->endpoint,
            'extension' => $this->extension,
            'context' => $this->context,
            'priority' => $this->priority,
            'callerId' => $this->callerId,
            'variables' => $this->variables,
        ];
    }
}