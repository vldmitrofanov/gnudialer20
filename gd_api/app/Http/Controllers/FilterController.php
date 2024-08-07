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

    private function makeDefaultFilters($campaign):void {
        $filters = \App\Models\Filter::$defaultFilters;
        foreach ($filters as $filter ){
            $campaign->filters()->save(\App\Models\Filter::create([
                'filter' => $filter['filter'],
                'enabled' => $filter['enabled'],
                'position' => $filter['position']
            ]));
        }
    }

    public function create(Request $request){
        $request->validate([
            'campaign_id' => 'required'
        ]);
        $campaign = \App\Models\Campaign::with(['filters'])->findOrFail($request->campaign_id);
        if($campaign->filters->count() == 0){
            $this->makeDefaultFilters($campaign);
        }
        if(!empty($request->filter)){
            $position = $campaign->filters->count() + 1;
            $campaign->filters()->save(\App\Models\Filter::create([
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
