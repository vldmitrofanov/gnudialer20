<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use App\Http\Resources\DispositionReportResource;
use App\Http\Resources\QueueResource;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Auth;
use App\Http\Resources\UserAgentResource;
use App\Models\Campaign;
use App\Models\Disposition;
use Illuminate\Support\Carbon;

class DispositionController extends Controller
{
    public function index(Request $request)
    {
        $start_date = !empty($request->start_date) ? Carbon::parse($request->start_date) : null;
        $perPage = !empty($request->perPage) ? $request->perPage : null;
        $end_date = !empty($request->end_date) ? Carbon::parse($request->end_date) : null;
        $campaign_id = !empty($request->campaign_id) ?$request->campaign_id : null;
        $orderBy = !empty($request->orderBy) ? 'dispositions.'.$request->orderBy : 'dispositions.id';
        $order = !empty($request->order) ? $request->order : 'desc';
        $agents = null;
        if (!empty($request->agents)) {
            if (is_array($request->agents)) {
                $agents = $request->agents;
            } else {
                $agents = explode(",", $request->agents);
            }
        }
        $dispositions = null;
        if (isset($request->dispositions)) {
            if (is_array($request->dispositions)) {
                $dispositions = $request->dispositions;
            } else {
                $dispositions = explode(",", $request->dispositions);
            }
        }
        $results = Disposition::select("dispositions.*")
            ->with(['agent','campaign','agent.user'])
            ->where('call_ended', 1);

        if ($start_date !== null) {
            $results = $results->where('start', '>=', $start_date->toDateTimeString());
        }
        if ($end_date !== null) {
            $results = $results->where('end', '<=', $end_date->toDateTimeString());
        }
        if ($campaign_id !== null) {
            $results = $results->where('campaign_id',  $campaign_id);
        } elseif(!empty($agents)){
            $results = $results->whereIn('agent_id',$agents);
        }
        if(!empty($dispositions)){
            $results = $results->whereIn('disposition',$dispositions);
        }

        $results = $results->orderBy($orderBy, $order)->paginate($perPage);
        return DispositionReportResource::collection($results);
    }
}
