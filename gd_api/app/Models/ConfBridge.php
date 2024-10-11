<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class ConfBridge extends Model
{
    use HasFactory;

    protected $fillable = [
        'agent_id',
        'bridge_id',
        'server_id',
        'online',
        'available',
        'pause',
        'agent_channel_id',
        'agent_channel'
    ];

    public function queues()
    {
        return $this->belongsToMany(Queue::class, 'conf_bridge_queue');
    }

    public function agent()
    {
        return $this->belongsTo(Agent::class);
    }

    public function server()
    {
        return $this->belongsTo(Server::class);
    }
}
