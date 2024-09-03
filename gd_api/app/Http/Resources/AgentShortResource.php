<?php

namespace App\Http\Resources;

use Illuminate\Http\Request;
use Illuminate\Http\Resources\Json\JsonResource;

class AgentShortResource extends JsonResource
{
    /**
     * Transform the resource into an array.
     *
     * @return array<string, mixed>
     */
    public function toArray(Request $request): array
    {
        return [
            'name' => '['.$this->id.'] '.$this->user->first_name . ' ' .  $this->user->last_name,
            'id' => $this->id,
            'server_is' => $this->server_id
        ];
    }
}
