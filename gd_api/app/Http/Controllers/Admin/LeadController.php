<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use App\Models\CampaignFormSchema;
use Illuminate\Support\Facades\DB;

class LeadController extends Controller
{
    public function index(Request $request)
    {
        $request->validate([
            'campaign' => 'required',
        ]);
        $campaign = $request->campaign;
        $order = $request->order? $request->order :'desc';
        $table_name = 'campaign_' . $campaign;
        $perPage = $request->perPage?$request->perPage:30;
        $orderBy = $request->orderBy?$request->orderBy:'id';
        $leads = DB::table($table_name);
        $qFieldName = $request->filterBy;
        $q = $request->filter;
        $qCondition = $request->filterCondition;
        if(!empty($qFieldName) && !empty($q)) {
        switch($qCondition) {
            case "=":
            case ">=":
            case "<=":  
            case "<":
            case ">":       
            case "!=":
                $leads = $leads->where($qFieldName, $qCondition, $q);
                break;
        }
    }
        $leads = $leads->orderBy($orderBy, $order)->paginate($perPage);

        $schema = CampaignFormSchema::where('table_name', $table_name)->first();
        return response()->json([
            'leads' => $leads,
            'schema' => $schema?->schema
        ]);
    }
}
