<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use App\Http\Resources\AdminCampaignResource;
use Illuminate\Http\Request;

class CampaignController extends Controller
{
    public function index(Request $request)
    {
        $perPage = $request->per_page?$request->per_page:15;
        $orderBy = $request->order_by?'campaigns.' . $request->order_by:'campaigns.name';
        $order = $request->order?$request->order:'asc';
        $campaigns = \App\Models\Campaign::with(['queues','queues.server']);

        $campaigns = $campaigns->orderBy($orderBy,$order)->paginate($perPage);
        return AdminCampaignResource::collection($campaigns);
    }

    public function show($campaign_id,Request $request)
    {
        $campaign = \App\Models\Campaign::with(['queues.settings','queues.filters','queues.server'])->findOrFail($campaign_id);

        return new AdminCampaignResource($campaign);
    }
}
