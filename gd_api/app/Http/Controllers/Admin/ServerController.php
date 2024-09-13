<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use App\Http\Resources\AdminCampaignResource;
use App\Http\Resources\ServerResource;
use Illuminate\Http\Request;

class ServerController extends Controller
{
    public function index(Request $request)
    {
        $servers = \App\Models\Server::orderBy('name')->get();
        return ServerResource::collection($servers);
    }
}
