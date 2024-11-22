<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Queue extends Model
{
    use HasFactory;
    const STATUS_ACTIVE=1;
    const STATUS_INACTIVE=0;

    const DIAL_METHOD_RATIO=6;
    const DIAL_METHOD_MANUAL=1;
    const DIAL_METHOD_INBOUND=2;
    const DIAL_METHOD_CLOSER=3;

    protected $fillable = [
        'campaign_id',
        'server_id',
        'status',
        'settings',
        'dial_method'
    ];

    protected $casts = [
        'settings' => 'array'
    ];

    public function agents()
    {
        return $this->belongsToMany(Agent::class);
    }

    public function server()
    {
        return $this->belongsTo(Server::class);
    }

    public function campaign()
    {
        return $this->belongsTo(Campaign::class);
    }

    public function queueSettings()
    {
        return $this->hasMany(Setting::class);
    }

    public function filters()
    {
        return $this->hasMany(Filter::class)->orderBy('position', 'asc');
    }

    public function confBridges()
    {
        return $this->belongsToMany(ConfBridge::class, 'conf_bridge_queue');
    }

    public function threeWays() 
    {
        return $this->hasMany(ThreeWay::class);
    }
}
