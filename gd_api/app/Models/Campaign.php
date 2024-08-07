<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Campaign extends Model
{
    use HasFactory;

    public function queues()
    {
        return $this->hasMany(Queue::class);
    }

    public function settings()
    {
        return $this->hasMany(CampaignSetting::class);
    }

    public function filters()
    {
        return $this->hasMany(Filter::class);
    }
}
