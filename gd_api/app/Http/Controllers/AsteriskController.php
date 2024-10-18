<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Services\AsteriskAMIService;
use App\Services\AsteriskARIService;
use Illuminate\Support\Facades\Log;
use Illuminate\Support\Facades\DB;

class AsteriskController extends Controller
{
    protected $amiService;
    protected $ariService;

    public function __construct(AsteriskAMIService $amiService, AsteriskARIService $ariService)
    {
        $this->amiService = $amiService;
        $this->ariService = $ariService;
    }

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
        /*
        try {
            $this->amiService->setServer($serverId);
            $response = $this->amiService->setAgentPause($queue, $agent, $pause);
            return response()->json(['message' => 'Agent pause status updated successfully', 'data' => $response], 200);
        } catch (\Exception $e) {
            return response()->json(['error' => $e->getMessage()], 500);
        }*/
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
        "Action: UserEvent\r\nUserEvent: SetOnWait\r\nHeader: Agent: " . $agent;
        $this->amiService->setServer($serverId);
        $status = $this->amiService->sendCommand($request->action . "\r\n\r\n", "\r\n");
        return response()->json(['message' => 'Agent available status updated successfully', 'data' => $brigde], 200);
    }

    /**
     * Get an agent's status regarding a queue.
     */
    public function getAgentStatus(Request $request)
    {
        $request->validate([
            'agent' => 'required',
            'queue' => 'nullable|string',
            'server_id' => 'required|integer'
        ]);

        $agent = $request->input('agent');
        $queue = $request->input('queue');
        $serverId = $request->input('server_id');
        $ariBridge = null;
        $channels = [];
        $brigde = \App\Models\ConfBridge::where('agent_id', $agent)->where('server_id', $serverId)->first();
        if (!empty($brigde)) {
            $this->ariService->setServer($serverId);
            if (!empty($brigde->bridge_id)) {
                $ariBridge = $this->ariService->getBridgeById($brigde->bridge_id);
            }
            if (empty($ariBridge)) {
                $ariBridges = $this->ariService->getAllBridges();
                foreach ($ariBridges as $v) {
                    if ($v['name'] == $brigde->id) {
                        $ariBridge = $v;
                        $brigde->bridge_id = $v['id'];
                    }
                }
            }
            if (!empty($ariBridge)) {
                if (!empty($ariBridge['channels'])) {
                    if (sizeof($ariBridge['channels']) > 1) {
                        $brigde->available = 0;
                    } else {
                        $brigde->available = 1;
                    }
                    foreach ($ariBridge['channels'] as $chanId) {
                        $chan = $this->ariService->getChannelById($chanId);
                        if ($chan && $chan['caller']['number'] == $agent) {
                            $brigde->agent_channel = $chan['name'];
                            $brigde->agent_channel_id = $chan['id'];
                        } elseif ($chan) {
                            $channels[] = $chan;
                        }
                    }
                }
            }
            $brigde->update();
        }
        return response()->json(['data' => $brigde, 'channels' => $channels], 200);
        /*
        try {
            $this->amiService->setServer($serverId);
            $status = $this->amiService->getAgentStatus($queue, $agent);
            return response()->json(['status' => $status], 200);
        } catch (\Exception $e) {
            return response()->json(['error' => $e->getMessage()], 500);
        }*/
    }

    public function callHangup(Request $request)
    {
        $request->validate([
            'channel' => 'required|string',
            'server_id' => 'required'
        ]);
        $serverId = $request->input('server_id');
        $this->amiService->setServer($serverId);
        $status = $this->amiService->channelHangup($request->channel);
        return response()->json(['status' => $status], 200);
    }

    public function userAction(Request $request)
    {
        $request->validate([
            'action' => 'required|string',
            'server_id' => 'required'
        ]);

        $serverId = $request->input('server_id');
        $this->amiService->setServer($serverId);
        $status = $this->amiService->sendCommand($request->action . "\r\n\r\n", "\r\n");
        return response()->json(['status' => $status], 200);
    }

    public function bridge3way(Request $request)
    {
        $request->validate([
            'three_way_id' => 'required',
            'server_id' => 'required',
            'bridge' => 'required',
            'lead_id' => 'nullable'
        ]);
        $bridge = $request->bridge;
        $threeWayId = $request->three_way_id;
        $threeWay = \App\Models\ThreeWay::findOrFail($threeWayId);
        $serverId = $request->server_id;
        $leadId = $request->lead_id;

        $command = "Action: Originate\r\n";
        $command .= "Channel: " . str_replace('_EXTEN_', $threeWay->extension, $threeWay->trunk) . "\r\n";
        //$command .= "Channel: Local/{$threeWay->extension}@{$threeWay->context}\r\n";
        $command .= "Exten: s\r\n";  // The third party's SIP channel
        $command .= "Context: $threeWay->context\r\n";  // The dialplan context
        $command .= "Priority: 1\r\n";  // Priority in dialplan
        $command .= "CallerID: $threeWay->caller_id\r\n";
        $command .= "Timeout: 30000\r\n";  // Timeout in milliseconds
        $command .= "ActionID: dialThirdParty\r\n";
        $command .= "Variable: CONF_BRIDGE_ID=" . $bridge . "\r\n";
        $command .= "Variable: LEAD_ID=" . $leadId . "\r\n";
        $command .= "Variable: CONNECTION_TYPE=THREE_WAY\r\n";
        $command .= "Async: true\r\n\r\n";
        //Log::info("Dialing extension: {$threeWay->extension}");
        $this->amiService->setServer($serverId);
        $result = $this->amiService->sendCommand($command, "Response: Success|Response: Error");
        if (strpos(serialize($result), 'Response: Success') !== false) {
            //$status =  $this->amiService->joinBridge($bridge, $result['channel']);
            return response()->json(['status' => 'OK'], 200);
        } else {
            return response()->json(['status' => null], 422);
        }
    }

    public function callLeadByID(Request $request)
    {
        $request->validate([
            'agent' => 'required',
            'queue' => 'required|string',
            'server_id' => 'required|integer',
            'lead_id' => 'required',
            'bridge' => 'required',
        ]);
        $campaignCode = $request->queue;
        $bridge = \App\Models\ConfBridge::findOrFail($request->bridge);
        $campaign = \App\Models\Campaign::where('code', $campaignCode)->first();
        if (empty($campaign)) {
            return response()->json(['message' => 'Campaign not found'], 422);
        }
        $leadId = $request->lead_id;
        $agent = $request->agent;
        $serverId = $request->server_id;
        $table_name = 'campaign_' . $campaignCode;
        $lead = DB::table($table_name)->where('id', $leadId)->where('disposition', '!=', 8);
        $lead = $lead->first();
        if (empty($lead)) {
            return response()->json(['message' => 'Lead not found'], 422);
        }
        $queue = null;
        foreach ($campaign->queues as $q) {
            if ($q->server_id == $serverId) {
                $queue = $q;
                break;
            }
        }
        if (empty($queue)) {
            return response()->json(['message' => 'Queue not found'], 422);
        }
        $dialNumber = $lead->phone;
        $confBridgeId = $bridge->id;
        $dialprefix = null;
        $cid = null;
        $trunk = null;

        // Loop through the queue settings
        foreach ($queue->queueSettings as $setting) {
            if ($setting->parameter === 'dialprefix') {
                $dialprefix = $setting->value;
            }
            if ($setting->parameter === 'callerid') {
                $cid = $setting->value;
            }
            if ($setting->parameter === 'trunk') {
                $trunk = $setting->value;
            }
        }

        // Ensure that $trunk and $dialprefix are set before proceeding
        if ($trunk && $dialprefix) {
            $channel = str_replace('_EXTEN_', $dialprefix . $dialNumber, $trunk);
        } else {
            // Handle the case where trunk or dialprefix are missing
            throw new \Exception('Missing required settings: trunk or dialprefix');
        }

        $context = "join_confbridge";
        $exten = "s";
        $priority = "1";

        // Create the AMI command
        $amiCommand = "Action: Originate\r\n";
        $amiCommand .= "Channel: " . $channel . "\r\n";
        $amiCommand .= "Context: " . $context . "\r\n";
        $amiCommand .= "Exten: " . $exten . "\r\n";
        $amiCommand .= "Priority: " . $priority . "\r\n";
        $amiCommand .= "CallerID: " . $cid . "\r\n";
        $amiCommand .= "Timeout: 30000\r\n";
        $amiCommand .= "Variable: CONF_BRIDGE_ID=" . $confBridgeId . "\r\n";
        $amiCommand .= "Async: true\r\n\r\n";

        $this->amiService->setServer($serverId);
        $result = $this->amiService->sendCommand($amiCommand, "Response: Success|Response: Error");
        if (strpos(serialize($result), 'Response: Success') !== false) {
            //$status =  $this->amiService->joinBridge($bridge, $result['channel']);
            $lead->update([
                'agent' => $agent,
                'lastupdated' => DB::raw('NOW()')
            ]);
            return response()->json(['status' => 'OK'], 200);
        } else {
            return response()->json(['status' => null], 422);
        }
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
        $newConfBridgeId = $campaignCode . "_" . $leadId;
        $amiCommand = "Action: Redirect\r\n";
        $amiCommand .= "Channel: " . $customerChannel . "\r\n"; // Channel ID of the customer
        $amiCommand .= "Context: leave3way_bridge_context\r\n"; // ConfBridge dialplan context
        $amiCommand .= "Exten: s\r\n";
        $amiCommand .= "Priority: 1\r\n";
        $amiCommand .= "Variable: CONF_BRIDGE_ID=" . $newConfBridgeId . "\r\n";
        $amiCommand .= "\r\n";
        $this->amiService->setServer($serverId);
        $this->amiService->sendCommand($amiCommand, "\r\n\r\n");

        $amiCommand = "Action: Redirect\r\n";
        $amiCommand .= "Channel: " . $threewayChannel . "\r\n"; // Channel ID of the manager
        $amiCommand .= "Context: leave3way_bridge_context\r\n";
        $amiCommand .= "Exten: s\r\n";
        $amiCommand .= "Priority: 1\r\n";
        $amiCommand .= "Variable: CONF_BRIDGE_ID=" . $newConfBridgeId . "\r\n";
        $amiCommand .= "\r\n";

        $this->amiService->sendCommand($amiCommand, "\r\n\r\n");
        return response()->json(['status' => 'OK'], 200);
    }
}
