<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class ThreeWay extends Model
{
    use HasFactory;

    protected $fillable = [
        'name',
        'extension',
        'trunk',
        'context',
        'caller_id',
        'queue_id'
    ];

    public function queue()
    {
        return $this->belongsTo(Queue::class);
    }
}
