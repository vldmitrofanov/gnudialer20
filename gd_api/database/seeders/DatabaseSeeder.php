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
                'synced' => 0,
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
                    'pjsip' => [
                        'config' => [],
                        'content' => ''
                    ]
                ],              
            ]);
            $data = $agent->settings;
            $ext = $agent->id; //'9' . str_pad($server->id, 2, "0", STR_PAD_LEFT) . $agent->id;
            $data['sip']['extension'] = $ext;
            $sip_config = [
                'extension' => $ext,
                'type' => 'friend',
                'password' => $sip_passwd,               
                'context' => 'gnudialer-default',
                'auth' => 'auth'.$ext,
                'aors' => $ext,
                'disallow' => 'all',
                'allow' => 'ulaw',
                'allow2' => 'alaw',
                'allow3' => 'gsm',
                'callerid' => 'Agent <' . $ext . '>',
                'max_contacts' => 1
            ];
            $sip_text = convert_pjsip_to_text($sip_config);
            $data['pjsip']['config'] = $sip_config;
            $data['pjsip']['content'] = $sip_text;
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

            $sip_passwd2 = Str::random(20);
            $agent2 = Agent::create([
                'user_id' => $agentUser2->id,
                'server_id' => $server->id,
                'password' => '9009',
                'settings' => [
                    'pjsip' => [
                        'config' => [],
                        'content' => '',
                    ]
                ],
            ]);
            $data = $agent2->settings;
            $ext = $agent2->id;//'9' . str_pad($server->id, 2, "0", STR_PAD_LEFT) . $agent2->id;
            $data['pjsip']['extension'] = $ext;
            $sip_config = [
                        'extension' => $ext,
                        'type' => 'friend',
                        'password' => $sip_passwd2,                      
                        'context' => 'gnudialer-default',
                        'aors' => $ext,
                        'auth' => 'auth'.$ext,
                        'disallow' => 'all',
                        'allow' => 'ulaw',
                        'allow2' => 'alaw',
                        'allow3' => 'gsm',
                        'callerid' => 'Agent <' . $ext . '>',
                        'max_contacts' => 1
                    ];
            $sip_text = convert_pjsip_to_text($sip_config);
            $data['pjsip']['content'] = $sip_text;
            $data['pjsip']['config'] = $sip_config;
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
                'settings' => [
                    'musicclass' => 'default',
                    'strategy' => 'ringall',
                    'timeout' => '15',
                    'retry' => '5',
                    'wrapuptime' => '10',
                    'maxlen' => '0',
                    'servicelevel' => 60,
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
                $queue->queueSettings()->save(\App\Models\Setting::create([
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

            $closerCampaign = Campaign::create([
                'name' => 'CLOSER',
                'code' => 'CLOSER',
                'status' => Campaign::STATUS_ACTIVE
            ]);

            $closerQueue = Queue::create([
                'campaign_id' => $closerCampaign->id,
                'server_id' => $server->id,
                'status' => Queue::STATUS_ACTIVE,               
                'settings' => [
                    'musicclass' => 'default',
                    'strategy' => 'ringall',
                    'timeout' => '15',
                    'retry' => '5',
                    'wrapuptime' => '10',
                    'maxlen' => '0',
                    'servicelevel' => 60,
                    'announce-frequency' => '0',
                    'min-announce-frequency' => '15',
                    'periodic-announce-frequency' => '0',
                    'announce-round-seconds' => '10',
                ]
            ]);
            foreach ($settings as $setting ){
                if($setting['parameter'] == 'active'){
                    $closerQueue->queueSettings()->save(\App\Models\Setting::create([
                        'parameter' => $setting['parameter'],
                        'value' => false,
                    ]));
                } else {
                    $closerQueue->queueSettings()->save(\App\Models\Setting::create([
                        'parameter' => $setting['parameter'],
                        'value' => $setting['value'],
                    ]));
                }
            }
            $filters = \App\Models\Filter::$defaultFilters;
            foreach ($filters as $filter ){
                $closerQueue->filters()->save(\App\Models\Filter::create([
                    'filter' => $filter['filter'],
                    'enabled' => $filter['enabled'],
                    'position' => $filter['position']
                ]));
            }
        }
    }
   
}

/* 
more queue settings

musicclass=default
strategy=ringall
timeout=15
retry=5
wrapuptime=10
maxlen=0
servicelevel=60
announce-frequency=30
announce-holdtime=yes
announce-position=yes
monitor-type=MixMonitor
memberdelay=0
weight=0
periodic-announce=queue-marketing
periodic-announce-frequency=60
relative-periodic-announce=yes
announce-round-seconds=10
joinempty=yes
leavewhenempty=no
ringinuse=no
eventwhencalled=yes
eventmemberstatus=yes
*/
