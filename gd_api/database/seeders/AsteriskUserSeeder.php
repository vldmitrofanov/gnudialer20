<?php

namespace Database\Seeders;

use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;
use App\Models\User;
use Illuminate\Support\Facades\Hash;

class AsteriskUserSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        $users = User::where('email','asterisk1@example.com')->get();

        if($users->count() == 0) {
            $serverUser = User::create([
                'first_name' => 'Asterisk 1',
                'last_name' => 'Server',
                'email' => 'asterisk1@example.com',
                'password' => Hash::make('z*BqE9H7%FRcspFd')
            ]);
            $serverUser->save();

        }
    }
}
