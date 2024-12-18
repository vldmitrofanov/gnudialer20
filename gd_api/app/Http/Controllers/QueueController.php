<?php

namespace App\Http\Controllers;

use App\Http\Resources\QueueResource;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;
use App\Models\CampaignFormSchema;
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
            ->with(['campaign', 'threeWays', 'queueSettings'])
            ->get();
        return QueueResource::collection($results);
    }

    public function getNextLead($queue_id, Request $request)
    {
        $request->validate([
            'agent_id' => 'required',
        ]);
        $queue = Queue::findOrFail($queue_id);
        if ($queue->status < 1) {
            return response()->json(['message' => 'This campaign is not active. Status: ' . $queue->status], 422);
        }
        if ($queue->dial_method != Queue::DIAL_METHOD_MANUAL) {
            return response()->json(['message' => 'This campaign doesn\'t support manual dial method'], 422);
        }
        $table_name = "campaign_" . $queue->campaign->code;
        $stmt = DB::table($table_name);
        $sortedFilters = collect($queue->filters)
            ->where('enabled', 1)                   // Only take enabled filters
            ->sortBy('position')                    // Sort by 'position'
            ->pluck('filter')                       // Get only the 'filter' field
            ->filter()                              // Remove null or empty filters
            ->toArray();                            // Convert to array

        // Step 2: Build the raw query by concatenating all filters
        $rawQuery = 'disposition <> 8  AND ' . implode(' AND ', $sortedFilters);  // Combine filters with AND operator

        // Example SQL query using the concatenated filters
        $lead = $stmt
            ->whereRaw($rawQuery)
            ->inRandomOrder()
            ->first();
        if (!empty($lead)) {
            DB::table($table_name)
                ->where('id', $lead->id)  // Assuming the lead has an 'id' field
                ->update([
                    //'agent' => $request->agent_id,
                    'lastupdated' => DB::raw('NOW()'),  // Use the current timestamp
                ]);

            // Return the updated lead
            $schema = CampaignFormSchema::where('table_name', $table_name)->first();
            return response()->json([
                'lead' => $lead,
                'schema' => $schema?->schema
            ]);
        } else {
            return response()->json(['message' => 'No leads found'], 404);
        }
    }
}
