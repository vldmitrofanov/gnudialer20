<?php

namespace Database\Seeders;

use App\Models\Agent;
use App\Models\Campaign;
use App\Models\Queue;
use App\Models\User;
use App\Models\Server;
// use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;
use Illuminate\Support\Facades\Hash;
use Illuminate\Support\Str;

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

        }

        if(Server::all()->count() == 0) {
            $server = Server::create([
                'name' => 'Asterisk',
                'username' => 'manager',
                'secret' => 'mysecret123',
                'host' => '127.0.0.1',
            ]);

            $agentUser = User::create([
                'first_name' => 'Agent1',
                'last_name' => 'Test',
                'email' => 'agent1@example.com',
                'password' => Hash::make("9nol'9")
            ]);
            $agentUser->id = 2;
            $agentUser->save();

            $sip_passwd = Str::random(20);
            $agent = Agent::create([
                'user_id' => $agentUser->id,
                'server_id' => $server->id,
                'password' => '9009',
                'settings' => [
                    'sip' => [
                        'extension' => '',
                        'type' => 'friend',
                        'secret' => $sip_passwd,
                        'host' => 'dynamic',
                        'context' => 'from-internal',
                        'disallow' => 'all',
                        'allow' => 'ulaw',
                        'allow' => 'alaw',
                        'allow' => 'gsm',
                        'callerid' => '',
                    ]
                ],
            ]);
            $data = $agent->settings;
            $data['sip']['extension'] = $agent->id;
            $data['sip']['callerid'] = 'Agent <' . $agent->id . '>';
            $agent->settings = $data;
            $agent->save();

            $agentUser2 = User::create([
                'first_name' => 'Agent2',
                'last_name' => 'Test',
                'email' => 'agent2@example.com',
                'password' => Hash::make("9nol'9")
            ]);
            $agentUser2->id = 3;
            $agentUser2->save();

            $sip_passwd = Str::random(20);
            $agent2 = Agent::create([
                'user_id' => $agentUser2->id,
                'server_id' => $server->id,
                'password' => '9009',
                'settings' => [
                    'sip' => [
                        'extension' => '',
                        'type' => 'friend',
                        'secret' => $sip_passwd,
                        'host' => 'dynamic',
                        'context' => 'from-internal',
                        'disallow' => 'all',
                        'allow' => 'ulaw',
                        'allow' => 'alaw',
                        'allow' => 'gsm',
                        'callerid' => '',
                    ]
                ],
            ]);
            $data = $agent2->settings;
            $data['sip']['extension'] = $agent2->id;
            $data['sip']['callerid'] = 'Agent <' . $agent2->id . '>';
            $agent2->settings = $data;
            $agent2->save();

            $campaign = Campaign::create([
                'name' => 'Test1',
                'code' => 'test1',
                'status' => Campaign::STATUS_ACTIVE
            ]);

            $queue = Queue:: create([
                'campaign_id' => $campaign->id,
                'server_id' => $server->id,
                'status' => Queue::STATUS_ACTIVE,
                'synced' => 0,
                'settings' => [
                    'musicclass' => 'default',
                    'strategy' => 'ringall',
                    'timeout' => '15',
                    'retry' => '5',
                    'wrapuptime' => '10',
                    'maxlen' => '0',
                    'announce-frequency' => '0',
                    'min-announce-frequency' => '15',
                    'periodic-announce-frequency' => '0',
                    'announce-round-seconds' => '10',
                ]
            ]);

            $queue->agents()->attach($agent);
            $queue->agents()->attach($agent2);

            $settings = \App\Models\Setting::$defaultSettings;
            foreach ($settings as $setting ){
                $queue->settings()->save(\App\Models\Setting::create([
                    'parameter' => $setting['parameter'],
                    'value' => $setting['value'],
                ]));
            }

            $filters = \App\Models\Filter::$defaultFilters;
            foreach ($filters as $filter ){
                $queue->filters()->save(\App\Models\Filter::create([
                    'filter' => $filter['filter'],
                    'enabled' => $filter['enabled'],
                    'position' => $filter['position']
                ]));
            }
        }
    }
}
