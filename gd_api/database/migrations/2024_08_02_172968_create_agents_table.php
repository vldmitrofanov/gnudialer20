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
        Schema::create('agents', function (Blueprint $table) {
            $table->id();
            $table->foreignId('user_id')->nullable()->index()->constrained()->onDelete('cascade');
            $table->foreignId('server_id')->nullable()->index()->constrained()->onDelete('cascade');;
            $table->string('password');
            $table->json('settings')->nullable();
            $table->timestamps();
        });

        DB::statement('ALTER TABLE agents AUTO_INCREMENT = 10001;');
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('agents');
    }
};
