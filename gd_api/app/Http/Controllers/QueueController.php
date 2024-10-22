<?php

namespace App\Http\Controllers;

use App\Http\Resources\QueueResource;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;
use App\Models\Queue;
use Illuminate\Support\Facades\DB;

class QueueController extends Controller
{
    public function agentIndex(Request $request)
    {
        $request->validate([
            'server_id' => 'required',
            'agent_id' => 'required',
        ]);
        $agent_id = $request->agent_id;
        $results = Queue::where('server_id', $request->server_id)->whereHas('agents', function ($q) use ($agent_id) {
            $q->where('id', $agent_id);
        })
            ->with(['campaign', 'campaign.threeWays', 'queueSettings'])
            ->get();
        return QueueResource::collection($results);
    }

    public function getNextLead(Queue $queue, Request $request)
    {
        $request->validate([
            'agent_id' => 'required',
        ]);
        if ($queue->dial_method != Queue::DIAL_METHOD_MANUAL) {
            return response()->json(['message' => 'This campaign doesn\'t support manual dial method'], 422);
        }
        $table_name = "campaign_" . $queue->campaign->code;
        $stmt = DB::table($table_name)->where('disposition', '<>', 8);
        $sortedFilters = collect($queue->filters)
            ->where('enabled', 1)                   // Only take enabled filters
            ->sortBy('position')                    // Sort by 'position'
            ->pluck('filter')                       // Get only the 'filter' field
            ->filter()                              // Remove null or empty filters
            ->toArray();                            // Convert to array

        // Step 2: Build the raw query by concatenating all filters
        $rawQuery = implode(' ', $sortedFilters);  // Combine filters with AND operator

        // Example SQL query using the concatenated filters
        $lead = $stmt
            ->whereRaw($rawQuery)
            ->first();
        if (!empty($lead)) {
            DB::table($table_name)
                ->where('id', $lead->id)  // Assuming the lead has an 'id' field
                ->update([
                    //'agent' => $request->agent_id,
                    'lastupdated' => DB::raw('NOW()'),  // Use the current timestamp
                ]);

            // Return the updated lead
            return response()->json([
                'lead' => $lead,
                'message' => 'Lead assigned to agent successfully'
            ], 200);
        } else {
            return response()->json(['message' => 'No leads found'], 404);
        }
    }
}
