<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\Schema;
use Illuminate\Support\Facades\DB;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        Schema::create('conf_bridges', function (Blueprint $table) {
            $table->id();
            $table->foreignId('agent_id')->nullable()->constrained()->onDelete('cascade');
            $table->foreignId('server_id')->nullable()->constrained()->onDelete('cascade');
            $table->tinyInteger('online')->default(0);
            $table->tinyInteger('available')->default(0);
            $table->tinyInteger('pause')->default(0);
            $table->string('channel')->nullable();
            $table->timestamps();

            $table->unique(['agent_id', 'server_id']);
        });

        DB::statement('ALTER TABLE conf_bridges AUTO_INCREMENT = 1740;');
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('conf_bridges');
    }
};
