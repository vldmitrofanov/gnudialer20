<?php

namespace Database\Seeders;

use App\Models\User;
use App\Models\Server;
// use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;
use Hash;

class DatabaseSeeder extends Seeder
{
    /**
     * Seed the application's database.
     */
    public function run(): void
    {
        $users = User::all();

        if($users->count() == 0) {
            $admin = User::create([
                'first_name' => 'Asterisk',
                'last_name' => 'Admin',
                'email' => 'admin@example.com',
                'password' => Hash::make('1dva3')
            ]);
            $admin->id = 1;
            $admin->save();

            $server = Server::create([
                'name' => 'Asterisk',
                'username' => 'manager',
                'secret' => 'mysecret123',
                'host' => '127.0.0.1',
            ]);
        }
    }
}
