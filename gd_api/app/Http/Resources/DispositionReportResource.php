<?php

namespace App\Http\Resources;

use Carbon\Carbon;
use Illuminate\Http\Request;
use Illuminate\Http\Resources\Json\JsonResource;
use Illuminate\Support\Facades\Auth;

class DispositionReportResource extends JsonResource
{
    /**
     * Transform the resource into an array.
     *
     * @return array<string, mixed>
     */
    public function toArray(Request $request): array
    {
        //return parent::toArray($request);
        $user = Auth::user();
        $start = !empty($this->start)?Carbon::parse($this->start,'UTC')->tz($user->timezone):null;
        $end = !empty($this->end)?Carbon::parse($this->end,'UTC')->tz($user->timezone):null;
        $call_duration = '';
        if(!empty($start)&&!empty($end)){
            $call_duration_holder = $start->diffInSeconds($end);
            $call_duration = str_pad(floor($call_duration_holder/60),2,"0") .":". str_pad($call_duration_holder % 60,2,"0") . " min";
        }
        $agent = [];
        $campaign = [];
        if(!empty($this->agent)) {
            $agent = [
                'id' => $this->agent->id,
                'name' => $this->agent->user?->first_name . " " . $this->agent->user?->last_name
            ];
        }
        if(!empty($this->campaign)) {
            $campaign = [
                'id' => $this->campaign->id,
                'name' => $this->campaign->name,
                'code' => $this->campaign->code
            ];
        }
        return [
            'id' => $this->id,
            'agent_id' => $this->agent_id,
            'campaign_id' => $this->campaign_id,
            'call_duration' => $call_duration,
            'start' => !empty($start)?$start->format('m/d/Y h:i A'):'',
            'end' => !empty($end)?$end->format('m/d/Y h:i A'):'',
            'disposition' => $this->disposition,
            'call_ended' => $this->call_ended,
            'agent' => $agent,
            'campaign' => $campaign,
            'lead_id' => $this->lead_id
        ];
    }
}
