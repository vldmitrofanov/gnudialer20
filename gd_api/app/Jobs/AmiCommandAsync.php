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
    private $serverId, $payload, $endLine;
    /**
     * Create a new job instance.
     */
    public function __construct( $serverId, $payload, $endLine)
    {
        $this->payload = $payload;
        $this->endLine = $endLine;
        $this->serverId = $serverId;
    }

    /**
     * Execute the job.
     */
    public function handle(AsteriskAMIService $amiService): void
    {
        $amiService->setServer($this->serverId);
        $amiService->sendCommand($this->payload, $this->endLine);
    }
}
