<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use App\Http\Resources\AgentActiveBridgeResource;
use App\Http\Resources\AgentResource;
use App\Http\Resources\AgentShortResource;
use App\Models\ConfBridge;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Str;

class AgentController extends Controller
{
    public function index(Request $request)
    {
        $per_page = 30;
        $agents = \App\Models\Agent::select("agents.*")->with(['user'])->join('users', 'agents.user_id', 'users.id');
        if (!empty($request->server_id)) {
            $agents = $agents->where('agents.server_id', $request->server_id);
        }
        if (empty($request->short)) {
            $agents = $agents->with('server');
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

    public function create(Request $request)
    {
        $request->validate([
            'action' => 'required',
            'password' => 'required',
            'server_id' => 'required',
            'copy_agent_id' => 'nullable',
            'first_name' => 'required',
            'last_name' => 'required',
            'email' => 'required|email|unique:users,email'
        ]);
        $newPasswd = $request->password;
        $sip_passwd = Str::random(20);
        $newUser = \App\Models\User::create([
            'first_name' => ucfirst(strtolower($request->first_name)),
            'last_name' => ucfirst(strtolower($request->last_name)),
            'email' => strtolower($request->email),
        ]);
        $newUser->password = Hash::make($newPasswd);
        if ($request->action == 'copy') {
            if (empty($request->copy_agent_id)) {
                return response()->json(['success' => false, 'message' => 'copy_agent_id is required'], 422);
            }
            $agentFrom = \App\Models\Agent::findOrFail($request->copy_agent_id);
            $agentArray = $agentFrom->toArray();

            // Remove the 'id' field from the array
            unset($agentArray['id']);
            $newAgent = \App\Models\Agent::create($agentArray);
            $newUser->agents()->save($newAgent);
            $data = $newAgent->settings;
            $ext = $newAgent->id; //'9' . str_pad($server->id, 2, "0", STR_PAD_LEFT) . $newAgent->id;
            $data['sip']['extension'] = $ext;
            $sip_config = [
                'extension' => $ext,
                'type' => 'friend',
                'password' => $sip_passwd,               
                'context' => 'gnudialer-default',
                'auth' => 'auth'.$ext,
                'aors' => $ext,
                'disallow' => 'all',
                'allow' => 'ulaw',
                'allow2' => 'alaw',
                'allow3' => 'gsm',
                'callerid' => 'Agent <' . $ext . '>',
                'max_contacts' => 1
            ];
            $sip_text = convert_pjsip_to_text($sip_config);
            $data['pjsip']['config'] = $sip_config;
            $data['pjsip']['content'] = $sip_text;
            $newAgent->settings = $data;
            $newAgent->save();
        }
        return new \App\Http\Resources\UserResource($newUser);
    }

    public function getActiveConfBridges(Request $request) {
        $res = ConfBridge::where('online',1)
            ->with(['agent', 'agent.queues', 'agent.queues.campaign', 'agent.user']);
        if(!empty($request->server_id)){
            $res = $res->where('server_id',$request->server_id);
        }
        $res = $res->orderBy('agent_id','asc')->get();
        return AgentActiveBridgeResource::collection($res);
    }
}
