<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Server extends Model
{
    use HasFactory;
    protected $fillable = [
        'name',
        'username',
        'secret',
        'host',
        'synced'
    ];

    public function canpaigns()
    {
        return $this->hasMany(Campaign::class);
    }
}
