<?php 

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Services\AsteriskAMIService;

class AsteriskController extends Controller
{
    protected $amiService;

    public function __construct(AsteriskAMIService $amiService)
    {
        $this->amiService = $amiService;
    }

    /**
     * Set an agent's pause status.
     */
    public function setAgentPause(Request $request)
    {
        $request->validate([
            'agent' => 'required',
            'queue' => 'required|string',
            'pause' => 'required|boolean',
            'server_id' => 'required|integer'
        ]);

        $agent = $request->input('agent');
        $queue = $request->input('queue');
        $pause = $request->input('pause');
        $serverId = $request->input('server_id');

        try {
            $this->amiService->setServer($serverId);
            $response = $this->amiService->setAgentPause($queue, $agent, $pause);
            return response()->json(['message' => 'Agent pause status updated successfully', 'response' => $response], 200);
        } catch (\Exception $e) {
            return response()->json(['error' => $e->getMessage()], 500);
        }
    }

    /**
     * Get an agent's status regarding a queue.
     */
    public function getAgentStatus(Request $request)
    {
        $request->validate([
            'agent' => 'required|string',
            'queue' => 'required|string',
            'server_id' => 'required|integer'
        ]);

        $agent = $request->input('agent');
        $queue = $request->input('queue');
        $serverId = $request->input('server_id');

        try {
            $this->amiService->setServer($serverId);
            $status = $this->amiService->getAgentStatus($queue, $agent);
            return response()->json(['status' => $status], 200);
        } catch (\Exception $e) {
            return response()->json(['error' => $e->getMessage()], 500);
        }
    }
}

