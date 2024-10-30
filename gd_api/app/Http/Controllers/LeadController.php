<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;
use App\Models\Agent;
use App\Models\CampaignFormSchema;
use App\Models\Disposition;
use Illuminate\Support\Facades\DB;
use App\Models\Campaign;

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
        if (!is_array($data)) {
            abort(422);
        }
        if (isset($data['phone'])) {
            unset($data['phone']);
        }
        $campaignCode = $request->campaign;
        $leadId = $request->lead['id'];
        $agentId = $request->agent;
        $dispo = $request->disposition;
        $now_string = \Carbon\Carbon::now()->toDateTimeString();
        $table_name = 'campaign_' . $campaignCode;
        unset($data['id']);
        $data['lastupdated'] = $now_string;
        $data['agent'] = $agentId;
        $data['disposition'] = $dispo;
        if (!empty($request->cb_datetime)) {
            $data['cb_datetime'] = \Carbon\Carbon::parse($request->cb_datetime)->toDateTimeString();
        }
        DB::table($table_name)->where('id', $leadId)->update($data);
        $campaign = Campaign::where('code', $campaignCode)->first();
        Disposition::where('call_ended', 0)
            ->where('agent_id', $agentId)
            ->where('lead_id', $leadId)
            ->where('campaign_id', $campaign->id)
            ->update([
                'disposition' => $dispo,
                'end' => $now_string,
                'call_ended' => 1
            ]);
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

    public function callbacks(Request $request)
    {
        $request->validate([
            'campaign' => 'required',
            'server_id' => 'nullable',
            'phone' => 'nullable',
        ]);
        $campaign = $request->campaign;
        $serverId = $request->server_id;
        $userId = $request->user()->id;

        $agent = Agent::whereHas('server', function ($q1) use ($serverId) {
            $q1->where('id', $serverId);
        })->whereHas('user', function ($q2) use ($userId) {
            $q2->where('id', $userId);
        })->first();

        if (empty($agent)) {
            abort(401);
        }

        $table_name = 'campaign_' . $campaign;
        $leads = DB::table($table_name)->where('agent', $agent->id)->where('disposition','0')->whereRaw('cb_datetime > NOW()');
        $leads = $leads->paginate(500);
        $schema = CampaignFormSchema::where('table_name', $table_name)->first();
        return response()->json([
            'leads' => $leads,
            'schema' => $schema?->schema
        ]);
    }
}
