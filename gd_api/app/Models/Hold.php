<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Hold extends Model
{
    protected $table = 'holds';

    // Specify the primary key field (use your custom ID column)
    protected $primaryKey = 'uid';

    // Disable auto-incrementing if the primary key is not auto-incremented
    public $incrementing = false;

    // Set the primary key type (use 'string' if the ID is not an integer)
    protected $keyType = 'string';

    protected $fillable = [
        'uid',
        'channel',
        'agent_id'
    ];
}
