<?php

namespace Database\Seeders;

use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;

class TestCampaignSchemaSeeder extends Seeder
{
    /**
     * Run the database seeds.
     */
    public function run(): void
    {
        $table = 'campaign_test1';
        $helper = new \App\Models\CampaignFormSchema();
        $myTable = \App\Models\CampaignFormSchema::where('table_name', $table)->first();
        if(empty($myTable)){
            $myTable =\App\Models\CampaignFormSchema::create([
                'table_name' => $table,
                'schema' => []
            ]);
        }
        $columns = $helper->getTableColumns($table);
        $schema = $helper->makeSchema($columns);
        $myTable->schema = $schema;
        $myTable->update();
    }
}
