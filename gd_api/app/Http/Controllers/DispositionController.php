<?php

namespace App\Http\Controllers;

use App\Http\Resources\QueueResource;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;
use App\Models\Campaign;
use App\Models\Disposition;

class DispositionController extends Controller
{
    public function create(Request $request)
    {
        $request->validate([
            'campaign_code' => 'required|string',
            'agent_id' => 'required',
            'lead_id' => 'required',
        ]);
        $campaign = Campaign::where('code',$request->campaign_code)->first();
        Disposition::create([
            'agent_id' => $request->agent_id,
            'campaign_id' => $campaign->id,
            'lead_id' => $request->lead_id,
            'start' => \Carbon\Carbon::now()->toDateTimeString()
        ]);
        return response()->json(['success' => true]);
    }

}
