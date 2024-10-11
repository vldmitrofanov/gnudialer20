<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Services\AsteriskAMIService;
use App\Services\AsteriskARIService;
use Illuminate\Support\Facades\Log;

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
            'queue' => 'required|string',
            'pause' => 'required|boolean',
            'server_id' => 'required|integer'
        ]);

        $agent = $request->input('agent');
        $queue = $request->input('queue');
        $pause = $request->input('pause');
        $pause = $pause == 'true' ? 1 : 0;
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

    /**
     * Get an agent's status regarding a queue.
     */
    public function getAgentStatus(Request $request)
    {
        $request->validate([
            'agent' => 'required|string',
            'queue' => 'required|string',
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
            } else {
                $ariBridges = $this->ariService->getAllBridges();
                foreach ($ariBridges as $v) {
                    if ($v['name'] == $brigde->id) {
                        $ariBridge = $v;
                        $brigde->bridge_id = $v['id'];
                    }
                }
            }
            if(!empty($ariBridge)){
                if(sizeof($ariBridge['channels'])>1) {
                    $brigde->available = 0;
                }
                foreach($ariBridge['channels'] as $chanId) {
                    $chan = $this->ariService->getChannelById($chanId);
                    if($chan && $chan['caller']['number'] == $agent) {
                        $brigde->agent_channel = $chan['name'];
                        $brigde->agent_channel_id = $chan['id'];
                    } elseif($chan) {
                        $channels[] = $chan;
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
            'bridge' => 'required|string',
        ]);
        $bridge = $request->bridge;
        $threeWayId = $request->three_way_id;
        $threeWay = \App\Models\ThreeWay::findOrFail($threeWayId);
        $serverId = $request->server_id;
        $this->amiService->setServer($serverId);
        $command = "Action: Originate\r\n";
        $command .= "Channel: Local/{$threeWay->extension}@{$threeWay->context}\r\n";
        $command .= "Exten: $threeWay->extension\r\n";  // The third party's SIP channel
        $command .= "Context: $threeWay->context\r\n";  // The dialplan context
        $command .= "Priority: 1\r\n";  // Priority in dialplan
        $command .= "CallerID: $threeWay->caller_id\r\n";
        $command .= "Timeout: 30000\r\n";  // Timeout in milliseconds
        $command .= "ActionID: dialThirdParty\r\n";
        $command .= "Async: true\r\n\r\n";
        Log::info("Dialing extension: {$threeWay->extension}");
        $result = $this->amiService->sendCommandAndGetChannell($command, "\r\n\r\n");
        if (!empty($result['channel'])) {
            $status =  $this->amiService->joinBridge($bridge, $result['channel']);
            return response()->json(['status' => $status, 'channel' => $result['channel']], 200);
        } else {
            return response()->json(['status' => null], 422);
        }
    }
}
