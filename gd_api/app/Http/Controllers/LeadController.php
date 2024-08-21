<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;
use Illuminate\Support\Facades\DB;

class LeadController extends Controller
{
    public function getLead(Request $request)
    {
        $request->validate([
            'campaign' => 'required',
            'lead_id' => 'required',
        ]);
        $campaign = $request->campaign;
        $leadId = $request->lead_id;
        $lead = DB::table('campaign_'.$campaign)->where('id',$leadId)->first();
        return response()->json([
            'lead' => $lead,
            'schema' => null
        ]);
    }
}
