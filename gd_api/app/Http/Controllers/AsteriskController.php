<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Jobs\MoveLineToNewBridge;

class AsteriskController extends Controller
{

    /**
     * Set an agent's pause status.
     */
    public function setAgentPause(Request $request)
    {
        $request->validate([
            'agent' => 'required',
            'pause' => 'required|boolean',
            'server_id' => 'required|integer'
        ]);

        $agent = $request->input('agent');
        $pause = $request->input('pause');
        $pause = $pause ? 1 : 0;
        $serverId = $request->input('server_id');
        $brigde = \App\Models\ConfBridge::where('agent_id', $agent)->where('server_id', $serverId)->first();
        if (empty($brigde)) {
            return response()->json(['message' => 'An error occured'], 422);
        }
        $brigde->pause = $pause;
        $brigde->update();
        return response()->json(['message' => 'Agent pause status updated successfully', 'data' => $brigde], 200);
    }

    public function setAgentAvailable(Request $request)
    {
        $request->validate([
            'agent' => 'required',
            'server_id' => 'required|integer'
        ]);
        $agent = $request->input('agent');
        $serverId = $request->input('server_id');
        $brigde = \App\Models\ConfBridge::where('agent_id', $agent)->where('server_id', $serverId)->first();
        if (empty($brigde)) {
            return response()->json(['message' => 'An error occured'], 422);
        }
        $brigde->available = 1;
        $brigde->update();
        return response()->json(['message' => 'Agent available status updated successfully', 'data' => $brigde], 200);
    }


    public function leave3way(Request $request)
    {
        $request->validate([
            'agent' => 'required',
            'queue' => 'required|string',
            'server_id' => 'required|integer',
            'lead_id' => 'required',
            'customer_channel' => 'required',
            'threeway_channel' => 'required',
        ]);
        $campaignCode = $request->queue;
        $leadId = $request->lead_id;
        $agent = $request->agent;
        $serverId = $request->server_id;
        $table_name = 'campaign_' . $campaignCode;
        $customerChannel = $request->customer_channel;
        $threewayChannel = $request->threeway_channel;
        $newConfBridgeId = $leadId; //$campaignCode . "_" . $leadId;
        /*
        $amiCommand = "Action: Redirect\r\n";
        $amiCommand .= "Channel: " . $customerChannel . "\r\n"; // Channel ID of the customer
        $amiCommand .= "Context: create_confbridge3w\r\n"; // ConfBridge dialplan context
        //$amiCommand .= "Exten: s\r\n";
        $amiCommand .= "Exten: " . $leadId . "\r\n";
        $amiCommand .= "Priority: 1\r\n";
        $amiCommand .= "Variable: NEW_CONF_BRIDGE_ID=" . $newConfBridgeId . "\r\n";
        $amiCommand .= "Variable: CAMPAIGN=" . $campaignCode . "\r\n";
        $amiCommand .= "Variable: LEAD_ID=" . $leadId . "\r\n";
        $amiCommand .= "Variable: AGENT=" . $agent . "\r\n";
        $amiCommand .= "\r\n";

        $amiCommand2 = "Action: Redirect\r\n";
        $amiCommand2 .= "Channel: " . $threewayChannel . "\r\n"; // Channel ID of the manager
        $amiCommand2 .= "Context: create_confbridge3w\r\n";
        $amiCommand2 .= "Exten: " . $leadId . "\r\n";
        $amiCommand2 .= "Priority: 1\r\n";
        $amiCommand2 .= "Variable: NEW_CONF_BRIDGE_ID=" . $newConfBridgeId . "\r\n";
        $amiCommand2 .= "Variable: CAMPAIGN=" . $campaignCode . "\r\n";
        $amiCommand2 .= "Variable: LEAD_ID=" . $leadId . "\r\n";
        $amiCommand2 .= "Variable: AGENT=" . $agent . "\r\n";
        $amiCommand2 .= "\r\n";
        $this->amiService->setServer($serverId);

        $this->amiService->sendCommand($amiCommand, "\r\n\r\n");
        $this->amiService->sendCommand($amiCommand2, "\r\n\r\n");
        */
        MoveLineToNewBridge::dispatch($customerChannel, $leadId, $newConfBridgeId, $campaignCode, $agent, $serverId);
        MoveLineToNewBridge::dispatch($threewayChannel, $leadId, $newConfBridgeId, $campaignCode, $agent, $serverId);

        return response()->json(['status' => 'OK'], 200);
    }

}
