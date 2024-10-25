<?php

namespace App\Jobs;

use Illuminate\Contracts\Queue\ShouldQueue;
use Illuminate\Foundation\Bus\Dispatchable;
use Illuminate\Foundation\Queue\Queueable;
use Illuminate\Queue\InteractsWithQueue;
use Illuminate\Queue\SerializesModels;

class AmiCommandAsync implements ShouldQueue
{
    use Queueable;
    private $amiHandle, $payload, $endLine;
    /**
     * Create a new job instance.
     */
    public function __construct($amiHandle, $payload, $endLine)
    {
        $this->amiHandle = $amiHandle;
        $this->payload = $payload;
        $this->endLine = $endLine;
    }

    /**
     * Execute the job.
     */
    public function handle(): void
    {
        $this->amiHandle->sendCommand($this->payload, $this->endLine);
    }
}
