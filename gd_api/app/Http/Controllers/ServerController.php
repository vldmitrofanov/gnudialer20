<?php

namespace App\Http\Controllers;

use App\Http\Resources\ServerResource;
use Illuminate\Http\Request;

class ServerController extends Controller
{
    public function show($server_id, Request $request) {
        $server = \App\Models\Server::findOrFail($server_id);
        return new ServerResource($server);
    }
}
