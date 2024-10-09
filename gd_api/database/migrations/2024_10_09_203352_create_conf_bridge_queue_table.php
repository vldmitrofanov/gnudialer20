<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('conf_bridge_queue', function (Blueprint $table) {
            $table->foreignId('conf_bridge_id')->constrained('conf_bridges')->onDelete('cascade');
            $table->foreignId('queue_id')->constrained('queues')->onDelete('cascade');
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('conf_bridge_queue');
    }
};
