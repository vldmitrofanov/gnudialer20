<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class CampaignFormSchema extends Model
{
    use HasFactory;
    protected $fillable = [
        'table_name',
        'schema'
    ];

    protected $casts = [
        'schema' => 'array'
    ];
}
