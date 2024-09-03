<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Queue extends Model
{
    use HasFactory;
    const STATUS_ACTIVE=1;
    const STATUS_INACTIVE=0;

    protected $fillable = [
        'campaign_id',
        'server_id',
        'status',
        'settings'
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

    public function settings()
    {
        return $this->hasMany(Setting::class);
    }

    public function filters()
    {
        return $this->hasMany(Filter::class)->orderBy('position', 'asc');
    }
}
