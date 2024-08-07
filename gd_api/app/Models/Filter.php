<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Filter extends Model
{
    use HasFactory;

    protected $fillable = [
        'enabled',
        'position',
        'campaign_id',
        'filter'
    ];

    static $defaultFilters = [
        0 => [
            'filter' => '((disposition > -6 AND disposition < 6) AND attempts < 4)',
            'enabled' => 1,
            'position' => 1
        ],
        1 => [
            'filter' => '((disposition > -6 AND disposition < 6) AND (attempts > 3 AND attempts < 8))',
            'enabled' => 0,
            'position' => 2
        ],
        2 => [
            'filter' => '((disposition > -6 AND disposition < 6) AND attempts > 7)',
            'enabled' => 0,
            'position' => 3
        ],
        3 => [
            'filter' => '(disposition = -3 OR disposition = 3)',
            'enabled' => 0,
            'position' => 4
        ],
        4 => [
            'filter' => '(disposition = 0)',
            'enabled' => 1,
            'position' => 5
        ],
        5 => [
            'filter' => '(disposition = -7)',
            'enabled' => 1,
            'position' => 6
        ],
    ];

    public function campaign()
    {
        return $this->belongsTo(Campaign::class);
    }
}
