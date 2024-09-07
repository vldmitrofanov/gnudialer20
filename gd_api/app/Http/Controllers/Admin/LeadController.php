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
        $order = 'desc';
        $table_name = 'campaign_' . $campaign;
        $leads = DB::table($table_name)->orderBy('id', $order)->paginate(30);
        $schema = CampaignFormSchema::where('table_name', $table_name)->first();
        return response()->json([
            'leads' => $leads,
            'schema' => $schema?->schema
        ]);
    }
}
