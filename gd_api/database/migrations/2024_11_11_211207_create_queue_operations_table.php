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
        Schema::create('queue_operations', function (Blueprint $table) {
            $table->id();
            $table->integer('calls')->default("0");
			$table->integer('totalcalls')->default("0");
			$table->integer('abandons')->default("0");
			$table->integer('totalabandons')->default("0");
			$table->integer('disconnects')->default("0");
			$table->integer('noanswers')->default("0");
			$table->integer('busies')->default("0");
			$table->integer('congestions')->default("0");
			$table->integer('ansmachs')->default("0");
            $table->integer('lines_dialing')->default("0");
            $table->foreignId('queue_id')->constrained('queues')->onDelete('cascade');
            $table->timestamps();
        });
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('queue_operations');
    }
};
