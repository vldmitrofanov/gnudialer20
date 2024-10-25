<?php

namespace App\Jobs;

use Illuminate\Contracts\Queue\ShouldQueue;
use Illuminate\Foundation\Bus\Dispatchable;
use App\Services\AsteriskAMIService;
use Illuminate\Foundation\Queue\Queueable;
use Illuminate\Queue\InteractsWithQueue;
use Illuminate\Queue\SerializesModels;

class AmiCommandAsync implements ShouldQueue
{
    use Queueable;
    private $amiService, $payload, $endLine;
    /**
     * Create a new job instance.
     */
    public function __construct( $serverId, $payload, $endLine)
    {
        $this->amiService = new AsteriskAMIService();
        $this->payload = $payload;
        $this->endLine = $endLine;
        $this->amiService->setServer($serverId);
    }

    /**
     * Execute the job.
     */
    public function handle(): void
    {
        $this->amiService->sendCommand($this->payload, $this->endLine);
    }
}
