<?php

namespace Database\Seeders;

use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;

class Server1Seeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        $data = [
            'ws' => [
                'user' => 'ws',
                'proto' => 'ws',
                'password' => 'secret123',
                'host' => '127.0.0.1:8088',
                'app_name' => 'my_app'
            ],
            'ari' => [
                'user' => 'apiuser',
                'proto' => 'http',
                'password' => 'secret123',
                'host' => '127.0.0.1:8088',
                'app_name' => 'my_app'
            ]
        ];
        \App\Models\Server::find(1)->update([
            'data' => $data
        ]);
    }
}
