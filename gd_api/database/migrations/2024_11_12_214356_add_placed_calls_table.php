<?php

use Illuminate\Database\Migrations\Migration;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Schema;

return new class extends Migration
{
    /**
     * Run the migrations.
     */
    public function up(): void
    {
        DB::statement("
            CREATE TABLE placed_calls (
                id CHAR(36) PRIMARY KEY DEFAULT (UUID()), 
                phone VARCHAR(20) NOT NULL,
                campaign VARCHAR(50) NOT NULL,
                leadid VARCHAR(50),
                callerid VARCHAR(50),
                usecloser BOOLEAN,
                dspmode VARCHAR(20),
                trunk VARCHAR(50),
                dialprefix VARCHAR(10),
                transfer VARCHAR(50),
                timeout_sec SMALLINT,
                api_id VARCHAR(50),
                server_id BIGINT UNSIGNED NOT NULL,
                called BOOLEAN DEFAULT false,
                answered BOOLEAN DEFAULT false,
                placed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        ");
    }

    /**
     * Reverse the migrations.
     */
    public function down(): void
    {
        Schema::dropIfExists('placed_calls');
    }
};
