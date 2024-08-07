<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Queue extends Model
{
    use HasFactory;

    protected $fillable = [
        'campaign_id',
        'server_id',
        'settings',
    ];

    protected $cast = [
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
}
