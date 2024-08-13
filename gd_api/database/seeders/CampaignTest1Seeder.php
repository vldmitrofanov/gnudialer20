<?php

namespace Database\Seeders;

use Illuminate\Database\Seeder;
use Illuminate\Support\Facades\DB;
use Faker\Factory as Faker;

class CampaignTest1Seeder extends Seeder
{
    public function run()
    {
        $faker = Faker::create();

        for ($i = 0; $i < 10000; $i++) {
            $rawPhone = $faker->phoneNumber;

            // Clean up the phone number: remove everything except digits
            $cleanPhone = preg_replace('/[^0-9]/', '', $rawPhone);
            DB::table('campaign_test1')->insert([
                'phone' => $cleanPhone,
                'title' => $faker->randomElement(['Mr', 'Ms', 'Mrs', 'Dr', 'Prof']),
                'name' => $faker->name,
                'address' => $faker->address,
                'city' => $faker->city,
                'state' => $faker->state,
                'zip' => $faker->postcode,
                'oc' => $faker->company,
                'paidamt' => $faker->randomFloat(2, 10, 500), // Random float between 10 and 500
                'paiddate' => $faker->date($format = 'Y-m-d', $max = 'now'),
                'comments' => $faker->sentence,
                'countynum' => $faker->randomNumber(3),
            ]);
        }
    }
}
