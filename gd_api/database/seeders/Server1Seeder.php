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
                'app_name' => 'gnudialer_stasis_app'
            ],
            'ari' => [
                'user' => 'apiuser',
                'proto' => 'http',
                'password' => 'secret123',
                'host' => '127.0.0.1:8088',
                'app_name' => 'gnudialer_stasis_app'
            ],
            'ami' => [
                'host' => '127.0.0.1',
                'port' => 5038,
                'username' => 'manager',
                'secret' => 'mysecret123',           
            ]
        ];
        \App\Models\Server::find(1)->update([
            'data' => $data
        ]);
    }
}
