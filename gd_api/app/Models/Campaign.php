<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Campaign extends Model
{
    use HasFactory;
    const STATUS_ACTIVE=1;
    const STATUS_INACTIVE=0;

    protected $fillable = [
        'name',        
        'status',
        'code'
    ];

    public function queues()
    {
        return $this->hasMany(Queue::class);
    }

    public function dispositions()
    {
        return $this->hasMany(Disposition::class)->orderById('desc');
    }
}
