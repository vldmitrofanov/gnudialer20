<?php

namespace App\Http\Controllers;

use App\Http\Resources\QueueResource;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;

class QueueController extends Controller
{
    public function agentIndex(Request $request) {
        $request->validate([
            'server_id' => 'required',
            'agent_id' => 'required',
        ]);
        $agent_id = $request->agent_id;
        $results = \App\Models\Queue::where('server_id', $request->server_id)->whereHas('agents', function($q) use($agent_id){
            $q->where('id',$agent_id);
        })
        ->with(['campaign', 'campaign.threeWays', 'campaign.settings'])
        ->get();
        return QueueResource::collection($results);
    }

}
