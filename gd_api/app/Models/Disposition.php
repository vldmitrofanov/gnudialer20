<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;

class Disposition extends Model
{
    use HasFactory;
    /*
	    System dispositions

		-8 == Burned (We give up)      ---o
		-7 == Disconnected Number         | -- Don't get called back
		-6 == Fax Machine              ---o
		-5 == Network Congestion ---o
		-4 == Busy                  |
		-3 == Voicemail             | -- Get called back ---o
		-2 == No Answer             |                       |
		-1 == Undefined*         ---o                       |
		                                                    |
		1 == Not Yet Called** -- Get called back -----------|
		                                                    |
		0 == Callback  ---o                                 |
		2 == No Answer    |                                 |
		3 == Voicemail    | -- Get called back -------------o
		4 == Busy         |
		5 == Fast Busy ---o
		6 == Fax Machine           ---o
		7 == Disconnected Number      |
		8 == Do Not Call              | -- Don't count as contacts
		9 == Invalid Lead             |
		10 == No Sale***           ---o
		11 == No Sale***    ---o These both... 
		12 == Sale          ---o count as contacts
    */

    const DSP_MACHINE_BURNED = -8;              // == Burned (We give up)      ---o
    const DSP_MACHINE_DISCONNECTED_NUMBER = -7; // == Disconnected Number         | -- Don't get called back
    const DSP_MACHINE_FAX = -6;                 // == Fax Machine              ---o
    const DSP_MACHINE_NETWORK_CONGESTION = -5; // == Network Congestion ---o
    const DSP_MACHINE_BUSY = -4;               // == Busy                  |
    const DSP_MACHINE_VOICEMAIL = -3;          // == Voicemail             | -- Get called back ---o
    const DSP_MACHINE_NO_ANSWER = -2;          // == No Answer             |                       |
    const DSP_MACHINE_UNDEFINED = -1;          // == Undefined*         ---o                       |
    //	                                                                                           |
    const DSP_NEW = 1;                        // == Not Yet Called** -- Get called back -----------|
    //	                                                                                           |
    const DSP_CALLBACK = 0;                   // == Callback  ---o                                 |
    const DSP_NO_ANSWER = 2;                  // == No Answer    |                                 |
    const DSP_VOICEMAIL =  3;                 // == Voicemail    | -- Get called back -------------o
    const DSP_BUSY = 4;                       // == Busy         |
    const DSP_FAST_BUSY = 5;                  // == Fast Busy ---o
    const DSP_FAX =    6;                     // == Fax Machine           ---o
    const DSP_DISCONNECTED_NUMBER = 7;        // == Disconnected Number      |
    const DSP_DNC = 8;                        // == Do Not Call              | -- Don't count as contacts
    const DSP_INVALID = 9;                    // == Invalid Lead             |
    const DSP_NO_SALE_NC = 10;                // == No Sale***            ---o
    const DSP_NO_SALE = 11;                   // == No Sale***    ---o These both... 
    const DSP_SALE = 12;                      // == Sale          ---o count as contacts

    protected $fillable = [
        'agent_id',
        'campaign_id',
        'disposition',
        'lead_id',
        'start',
        'end',
        'call_ended'
    ];

    public function campaign()
    {
        return $this->belongsTo(Campaign::class);
    }

    public function agent()
    {
        return $this->belongsTo(Agent::class);
    }
}
