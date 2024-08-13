<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Setting extends Model
{
    use HasFactory;

    protected $fillable = [
        'queue_id',
        'parameter',
        'value',
        'type'
    ];

    static $defaultSettings = [
        0 => ['parameter' => 'active', 'value' => 'true'],
        1 => ['parameter' => 'function', 'value' => 'predictive'],
        2 => ['parameter' => 'maxratio', 'value' => 2.0],
        3 => ['parameter' => 'maxabandons', 'value' => 3.0],
        4 => ['parameter' => 'maxlines', 'value' => 23],
        5 => ['parameter' => 'sleeptime', 'value' => 1000],
        6 => ['parameter' => 'tzearliest', 'value' => 9],
        7 => ['parameter' => 'tzlatest', 'value' => 21],
        8 => ['parameter' => 'timeout', 'value' => 20000],
        9 => ['parameter' => 'dspmode', 'value' => 'none'],
        10 => ['parameter' => 'trunk', 'value' => 'Zap/g1'],
        11 => ['parameter' => 'dialprefix', 'value' => 'none'],
        12 => ['parameter' => 'usecloser', 'value' => 'true'],
        13 => ['parameter' => 'callerid', 'value' => '9999999999'],
        14 => ['parameter' => 'filter', 'value' => 0],
        15 => ['parameter' => 'calls', 'value' => 0],
        16 => ['parameter' => 'abandons', 'value' => 0],
        17 => ['parameter' => 'totalcalls', 'value' => 0],
        18 => ['parameter' => 'totalabandons', 'value' => 0],
        19 => ['parameter' => 'closercam', 'value' => 'CLOSER'],
        20 => ['parameter' => 'talktime', 'value' => 0],
        21 => ['parameter' => 'totaltalktime', 'value' => 0],
        22 => ['parameter' => 'usecallback', 'value' => 'false'],
        23 => ['parameter' => 'usetzfilter', 'value' => 'false'],
        24 => ['parameter' => 'debug', 'value' => 'true'],
        25 => ['parameter' => 'skip', 'value' => 0],
        26 => ['parameter' => 'f_mode', 'value' => 0],
        27 => ['parameter' => 'f_areacode', 'value' => 0],
        28 => ['parameter' => 'f_areacode_prefix', 'value' => 0],
        29 => ['parameter' => 'f_zipcode', 'value' => 0],
        30 => ['parameter' => 'calltoday', 'value' => 0, 'type' => 'int'],
        31 => ['parameter' => 'usednc','value' => 'true' ],
        32 => ['parameter' => 'orderby','value' => 'id' ]
    ];

    public function queue()
    {
        return $this->belongsTo(Queue::class);
    }
}
