<?php
namespace App\GnuDialer\Extra;
class ARIRedirectPayload
{
    public $channel;    // SIP or PJSIP channel (e.g., PJSIP/1000)
    public $extension;   // Extension to dial (e.g., 100)
    public $context;     // Dialplan context (e.g., from-internal)
    public $priority;    // Dialplan priority (e.g., 1)
    public $variables;

    public function __construct($channel, $extension, $context, $priority,$variables)
    {
        $this->channel = $channel;
        $this->extension = $extension;
        $this->context = $context;
        $this->priority = $priority;
        $this->variables = array_map('strval', $variables);
    }

    public function toArray()
    {
        return [
            'extension' => $this->extension,
            'context' => $this->context,
            'priority' => $this->priority,
            'variables' => $this->variables,
        ];
    }
}