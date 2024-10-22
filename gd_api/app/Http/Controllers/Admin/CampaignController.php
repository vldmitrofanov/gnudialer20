<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use App\Http\Resources\AdminCampaignResource;
use Illuminate\Http\Request;
use App\Models\Queue;
use App\Models\Campaign;
use App\Models\Server;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;

class CampaignController extends Controller
{
    public function index(Request $request)
    {
        $perPage = $request->per_page ? $request->per_page : 15;
        $orderBy = $request->order_by ? 'campaigns.' . $request->order_by : 'campaigns.name';
        $order = $request->order ? $request->order : 'asc';
        $campaigns = Campaign::with(['queues', 'queues.server']);

        $campaigns = $campaigns->orderBy($orderBy, $order)->paginate($perPage);
        return AdminCampaignResource::collection($campaigns);
    }

    public function show($campaign_id, Request $request)
    {
        $campaign = Campaign::with(['queues.queueSettings', 'queues.filters', 'queues.server', 'queues.agents'])->findOrFail($campaign_id);

        return new AdminCampaignResource($campaign);
    }

    public function create(Request $request)
    {
        $request->validate([
            'name' => 'required|string',
            'code' => 'required|string',
            'table' => 'required|array',
            'servers' => 'required|array',
        ]);

        $servers = Server::whereIn('id', $request->servers)->get();

        if ($servers->count() == 0) {
            return response()->json(['success' => false, 'message' => 'servers were not found']);
        }

        $campaign = Campaign::create([
            'name' => $request->name,
            'code' => $request->code,
            'status' => Campaign::STATUS_ACTIVE
        ]);

        foreach ($servers as $server) {
            $queue = Queue::create([
                'campaign_id' => $campaign->id,
                'server_id' => $server->id,
                'status' => Queue::STATUS_ACTIVE,
                'settings' => [
                    'musicclass' => 'default',
                    'strategy' => 'ringall',
                    'timeout' => '15',
                    'retry' => '5',
                    'wrapuptime' => '10',
                    'maxlen' => '0',
                    'servicelevel' => 60,
                    'announce-frequency' => '0',
                    'min-announce-frequency' => '15',
                    'periodic-announce-frequency' => '0',
                    'announce-round-seconds' => '10',
                ]
            ]);

            $settings = \App\Models\Setting::$defaultSettings;
            foreach ($settings as $setting) {
                $queue->queueSettings()->save(\App\Models\Setting::create([
                    'parameter' => $setting['parameter'],
                    'value' => $setting['value'],
                ]));
            }

            $filters = \App\Models\Filter::$defaultFilters;
            foreach ($filters as $filter) {
                $queue->filters()->save(\App\Models\Filter::create([
                    'filter' => $filter['filter'],
                    'enabled' => $filter['enabled'],
                    'position' => $filter['position']
                ]));
            }
        }

        $tableName = 'campaign_' . $request->code;
        Schema::create($tableName, function (Blueprint $table) use ($request) {
            $table->id();
            foreach ($request->table as $fieldName => $fieldType) {
                $fieldName = strtolower($fieldName);
                $fieldName = preg_replace('/[^a-z0-9_]/', '', $fieldName);
                if ($fieldType === 'string') {
                    $table->string($fieldName)->nullable();
                } elseif ($fieldType === 'text') {
                    $table->text($fieldName)->nullable();
                } elseif ($fieldType === 'int') {
                    $table->integer($fieldName)->nullable();
                } elseif ($fieldType === 'date-time') {
                    $table->dateTime($fieldName)->nullable();
                }
            }
        });

        DB::statement("ALTER TABLE $tableName ADD UNIQUE (phone);");

        return response()->json(['success' => true, 'message' => 'Campaign sucessfully created']);
    }

    public function delete($campaign_id) {
        $campaign = Campaign::findOrFail($campaign_id);
        $tableName = 'campaign_' . $campaign->code;
        DB::statement("DROP TABLE $tableName;");
        foreach($campaign->queues as $queue) {
            $queue->delete();
        }
        $campaign->delete();
        return response()->json(['success' => true, 'message' => 'Campaign sucessfully deleted']);
    }
}
