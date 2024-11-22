<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Services\AsteriskAMIService;
use App\Services\AsteriskARIService;
use Illuminate\Support\Facades\Log;
use Illuminate\Support\Facades\DB;
use App\Jobs\MoveLineToNewBridge;
use App\GnuDialer\Extra\ARICallPayload;
use App\GnuDialer\Extra\ARIRedirectPayload;
use App\Jobs\AmiCommandAsync;
use App\Models\Disposition;

class AsteriskAriController extends Controller
{
    protected $ariService;

    public function __construct(AsteriskARIService $ariService)
    {
        $this->ariService = $ariService;
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
            'channel_id' => 'nullable',
            'channel_name' => 'required_without:channel_id',
            'server_id' => 'required'
        ]);
        $serverId = $request->input('server_id');
        $this->ariService->setServer($serverId);
        $chId = null;
        if(empty($request->channel_id)){
            $ch = $this->ariService->findChannelByName($request->channel_name);
            $chId = $ch['id'];
        } else {
            $chId = $request->channel_id;
        }
        if(empty($chId)) {
            return response()->json(['status' => 'Channel not found'], 400);
        }
        $status = $this->ariService->channelHangup($chId);
        return response()->json(['status' => $status], 200);
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
        $timeout = 60;

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
            if ($setting->parameter === 'timeout') {
                $timeout = $setting->value;
            }
        }

        // Ensure that $trunk and $dialprefix are set before proceeding
        if ($trunk && $dialprefix) {
            $channel = str_replace('_EXTEN_', $dialprefix . $dialNumber, $trunk);
        } else {
            // Handle the case where trunk or dialprefix are missing
            throw new \Exception('Missing required settings: trunk or dialprefix');
        }

        /*
        $context = "join_confbridge";
        $exten = "s";
        $priority = "1";

        $pl = new ARICallPayload(
            $channel,
            $exten,
            $context,
            $priority,
            $cid,
            ['CONF_BRIDGE_ID' => (string)$confBridgeId]
        );
        */
        $this->ariService->setServer($serverId);

        $chanVariables = [
            "LEADID" => (string) $lead->id,
            "CAMPAIGN"=> (string) $campaignCode,
            "DSPMODE" => '',
            "METHOD" => "manual",
            "ISTRANSFER" => "false",
            "AGENTID" => (string) $agent
        ];
        $jsonPayload = [
            "endpoint" => $channel,
            "extension" =>  $dialprefix . $dialNumber,
            "context" => "gdstasis",
            "priority" => 1,
            "callerId" => "~" . $campaignCode . "-" .$lead->id. "~",
            "timeout" => $timeout,
            "variables" => $chanVariables
        ];

        //$resp = $this->ariService->createChannel($pl);
        
        $resp = $this->ariService->manualCall($jsonPayload);

        DB::table($table_name)
            ->where('id', $leadId) // Assuming you have an `id` field for identifying the lead
            ->update([
                'agent' => $agent,
                'lastupdated' => DB::raw('NOW()')
            ]);
        if ($request->create_disposition) {
            Disposition::create([
                'agent_id' => $agent,
                'campaign_id' => $campaign->id,
                'lead_id' => $leadId,
                'start' => \Carbon\Carbon::now()->toDateTimeString()
            ]);
        }
        return response()->json(['status' => 'OK', 'response' => $resp], 200);
        // } else {
        //      return response()->json(['status' => null], 422);
        // }
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

        MoveLineToNewBridge::dispatch($customerChannel, $leadId, $newConfBridgeId, $campaignCode, $agent, $serverId);
        MoveLineToNewBridge::dispatch($threewayChannel, $leadId, $newConfBridgeId, $campaignCode, $agent, $serverId);

        return response()->json(['status' => 'OK'], 200);
    }

    public function toggleHold(Request $request)
    {
        $request->validate([
            'agent' => 'required',
            'queue' => 'nullable|string',
            'server_id' => 'required|integer',
            'lead_id' => 'nullable',
            'bridge_id' => 'nullable',
            'channel_id' => 'required',
            'action' => 'required'
        ]);

        $serverId = $request->server_id;
        $channelId = $request->channel_id;
        $action = $request->action;
        $putOnHold = $action == 'on' ? true : false;
        

        $this->ariService->setServer($serverId);
        if($putOnHold){
            $this->ariService->putChannelOnHold($channelId);
        } else {
            $this->ariService->releaseChannelFromHold($channelId);
        }

        return response()->json(['status' => 'OK'], 200);
    }

    
}
