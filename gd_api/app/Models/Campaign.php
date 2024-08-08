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
        'status'
    ];

    public function queues()
    {
        return $this->hasMany(Queue::class);
    }


}
