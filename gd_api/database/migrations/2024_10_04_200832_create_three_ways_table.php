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
        Schema::create('three_ways', function (Blueprint $table) {
            $table->id();
            $table->string('name')->nullable();
            $table->string('extension')->nullable();
            $table->string('trunk')->nullable();
            $table->string('context')->nullable();
            $table->string('caller_id')->nullable();
            $table->foreignId('campaign_id')->nullable()->index()->constrained()->onDelete('cascade');
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('three_ways');
    }
};
