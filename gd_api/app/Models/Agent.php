<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Agent extends Model
{
    use HasFactory;

    protected $fillable = [
        'user_id',
        'server_id',
        'password',
        'settings',
    ];

    protected $casts = [
        'settings' => 'array'
    ];

    public function queues()
    {
        return $this->belongsToMany(Queue::class);
    }

    public function user()
    {
        return $this->belongsTo(User::class);
    }

    public function server()
    {
        return $this->belongsTo(Server::class);
    }

    public function dispositions()
    {
        return $this->hasMany(Disposition::class)->orderById('desc');
    }
}
