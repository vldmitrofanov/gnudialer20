<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;

class QueueController extends Controller
{
    public function updateAgents($queue_id, Request $request) {
        $request->validate([
            'action' => 'required',
            'agent_ids' => 'required',
        ]);
        $agentIds = $request->agent_ids;
        if(empty($agentIds) || !is_array($agentIds)) {
            return response()->json(['success' => false, 'message' => 'Please provide list of agents'],422);
        }
        $queue = \App\Models\Queue::findOrFail($queue_id);
        if($request->action == 'remove') {
            $queue->agents()->detach($agentIds);
        } else {
            $queue->agents()->attach($agentIds);
        }
            
        return response()->json(['success' => true],200);
    }
}
