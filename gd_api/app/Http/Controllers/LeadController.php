<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;
use App\Models\CampaignFormSchema;
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
        $table_name = 'campaign_' . $campaign;
        $lead = DB::table($table_name)->where('id', $leadId)->first();
        $schema = CampaignFormSchema::where('table_name', $table_name)->first();
        return response()->json([
            'lead' => $lead,
            'schema' => $schema?->schema
        ]);
    }

    public function updateLead(Request $request) 
    {
        $request->validate([
            'lead' => 'required',
            'campaign' => 'required',
            'agent' => 'required',
            'disposition' => 'required',
            'cb_datetime' => 'nullable'
        ]);

        $data = $request->lead;
        if(!is_array($data)) {
            abort(422);
        }
        if(isset($data['phone'])){
            unset($data['phone']);
        }      
        $campaign = $request->campaign;
        $leadId = $request->lead['id'];
        $table_name = 'campaign_' . $campaign;
        unset($data['id']);
        $data['lastupdated'] = \Carbon\Carbon::now();
        $data['agent'] = $request->agent;
        $data['disposition'] = $request->disposition;
        if(!empty($request->cb_datetime)){
            $data['cb_datetime'] = \Carbon\Carbon::parse($request->cb_datetime)->toDateTimeString();
        }
        DB::table($table_name)->where('id', $leadId)->update($data);
        return response()->json([
            'message' => 'Resource updated successfully',
        ], 201);
    }

    public function searchLead(Request $request)
    {
        $request->validate([
            'campaign' => 'required',
            'lead_id' => 'nullable',
            'phone' => 'nullable',
        ]);
        $campaign = $request->campaign;
        $leadId = $request->lead_id;
        $phone = $request->phone;
        $table_name = 'campaign_' . $campaign;
        $lead = DB::table($table_name);
        if (!empty($leadId)) {
            $lead = $lead->where('id', $leadId);
        }
        if (!empty($phone)) {
            $lead = $lead->where('phone', $phone);
        }
        $lead = $lead->first();
        $schema = CampaignFormSchema::where('table_name', $table_name)->first();
        return response()->json([
            'lead' => $lead,
            'schema' => $schema?->schema
        ]);
    }
}
