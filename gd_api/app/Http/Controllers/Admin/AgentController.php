<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use App\Http\Resources\AgentResource;
use App\Http\Resources\AgentShortResource;
use Illuminate\Http\Request;

class AgentController extends Controller
{
    public function index(Request $request)
    {
        $per_page = 15;
        $agents = \App\Models\Agent::select("agents.*")->with(['user'])->join('users', 'agents.user_id', 'users.id');
        if (!empty($request->server_id)) {
            $agents = $agents->where('agents.server_id', $request->server_id);
        }
        $agents = $agents->orderBy('users.first_name')->orderBy('users.last_name');
        if (empty($request->paginate)) {
            $agents = $agents->get();
        } else {
            $agents = $agents->paginate($per_page);
        }
        if (!empty($request->short)) {
            return AgentShortResource::collection($agents);
        } else {
            return AgentResource::collection($agents);
        }
    }
}
