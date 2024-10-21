<?php

namespace App\Jobs;

use Illuminate\Contracts\Queue\ShouldQueue;
use Illuminate\Foundation\Bus\Dispatchable;
use Illuminate\Foundation\Queue\Queueable;
use Illuminate\Queue\InteractsWithQueue;
use Illuminate\Queue\SerializesModels;
use App\Services\AsteriskAMIService;

class MoveLineToNewBridge implements ShouldQueue
{
    use Queueable;
    protected $channel, $leadId, $newConfBridgeId, $campaignCode, $agent, $serverId;
    /**
     * Create a new job instance.
     */
    public function __construct($channel, $leadId, $newConfBridgeId, $campaignCode, $agent, $serverId)
    {
        $this->channel = $channel;
        $this->leadId = $leadId;
        $this->newConfBridgeId = $newConfBridgeId;
        $this->campaignCode = $campaignCode;
        $this->agent = $agent;
        $this->serverId = $serverId;
    }

    /**
     * Execute the job.
     */
    public function handle(AsteriskAMIService $amiService): void
    {
        $amiCommand = "Action: Redirect\r\n";
        $amiCommand .= "Channel: " . $this->channel . "\r\n"; // Channel ID of the customer
        $amiCommand .= "Context: create_confbridge3w\r\n"; // ConfBridge dialplan context
        $amiCommand .= "Exten: " . $this->leadId . "\r\n";
        $amiCommand .= "Priority: 1\r\n";
        $amiCommand .= "Variable: NEW_CONF_BRIDGE_ID=" . $this->newConfBridgeId . "\r\n";
        $amiCommand .= "Variable: CAMPAIGN=" . $this->campaignCode . "\r\n";
        $amiCommand .= "Variable: LEAD_ID=" . $this->leadId . "\r\n";
        $amiCommand .= "Variable: AGENT=" . $this->agent . "\r\n";
        $amiCommand .= "\r\n";
        $amiService->setServer($this->serverId);
        $amiService->sendCommand($amiCommand, "\r\n\r\n");
    }
}
