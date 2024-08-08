<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;

class FilterController extends Controller
{
    //

    /*
       ;:setting:filters:number:0:enable:true:string:((disposition > -6 AND disposition < 6) AND attempts < 4)
	   ;:setting:filters:number:1:enable:false:string:((disposition > -6 AND disposition < 6) AND (attempts > 3 AND attempts < 8))
	   ;:setting:filters:number:2:enable:false:string:((disposition > -6 AND disposition < 6) AND attempts > 7)
	   ;:setting:filters:number:3:enable:false:string:(disposition = -3 OR disposition = 3)
	   ;:setting:filters:number:4:enable:false:string:(disposition = 0)
	   ;:setting:filters:number:5:enable:false:string:(disposition = -7)
    */

    private function makeDefaultFilters(\App\Models\Queue $queue):void {
        $filters = \App\Models\Filter::$defaultFilters;
        foreach ($filters as $filter ){
            $queue->filters()->save(\App\Models\Filter::create([
                'filter' => $filter['filter'],
                'enabled' => $filter['enabled'],
                'position' => $filter['position']
            ]));
        }
    }

    public function create(Request $request){
        $request->validate([
            'campaign_id' => 'required',
            'server_id' => 'required'
        ]);
        $campaign_id = $request->campaign_id;
        $server_id = $request->server_id;

        $queue = \App\Models\Queue::with(['filters'])
            ->whereHas('campaign', function($q) use($campaign_id) {
                $q->where('id',$campaign_id);
            })->where('server_id', $server_id)
            ->first();

        if(empty($queue)){
            $queue = \App\Models\Queue::create([
                'campaign_id' => $campaign_id,
                'server_id' => $server_id,
                'status' => 1
            ]);
        }
        
        if($queue->filters->count() == 0){
            $this->makeDefaultFilters($queue);
        }

        if(!empty($request->filter)){
            $position = $queue->filters->count() + 1;
            $queue->filters()->save(\App\Models\Filter::create([
                'filter' => $request->filter,
                'enabled' => 1,
                'position' => $position
            ]));
        }
        return response()->json(['success'=>true]);
    }

    public function delete($filter_id){
        \App\Models\Filter::findOrFail($filter_id);
        return response()->json(['success'=>true]);
    }
}
