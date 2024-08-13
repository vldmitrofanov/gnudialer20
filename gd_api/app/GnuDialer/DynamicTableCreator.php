<?php
namespace App\GnuDialer;

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Support\Facades\DB;

class DynamicTableCreator
{
    public function createTable($campaign_id, $optionalColumns = []):void
    {
        $campaign = \App\Models\Campaign::findOrFail($campaign_id);
        $tableName = 'campaign_' . $campaign->code;
        Schema::create($tableName, function (Blueprint $table) use ($optionalColumns) {
            // Required fields
            $table->id('id');  // Primary Key
            $table->string('phone', 15)->index();

            // Optional fields (dynamically added)
            foreach ($optionalColumns as $column) {
                $type = $column['type'];
                $name = $column['name'];
                $nullable = $column['nullable'] ?? true;  // Default to true if not specified
                $default = $column['default'] ?? null;

                $columnDefinition = $table->$type($name);

                if ($nullable) {
                    $columnDefinition->nullable();
                }

                if ($default !== null) {
                    $columnDefinition->default($default);
                }
            }


            $table->integer('tzl')->nullable();
            $table->integer('tzh')->nullable();
            $table->integer('attempts')->nullable()->index();
            $table->integer('pickups')->nullable()->index();
            $table->integer('abandons')->nullable();
            $table->integer('disposition')->nullable()->index();
            $table->string('agent', 10)->nullable();
            $table->integer('closerdispo')->nullable();
            $table->string('closer', 10)->nullable();
            $table->string('subdispo', 10)->nullable();
            $table->timestamp('lastupdated')->nullable()->default(DB::raw('CURRENT_TIMESTAMP'));
            $table->timestamp('cb_datetime')->nullable()->default(DB::raw('CURRENT_TIMESTAMP'));
            $table->integer('wdayl')->nullable();
            $table->integer('wdayh')->nullable();
            $table->integer('satl')->nullable();
            $table->integer('sath')->nullable();
            $table->integer('sunl')->nullable();
            $table->integer('sunh')->nullable();
            $table->integer('holl')->nullable();
            $table->integer('holh')->nullable();

           
        });
    }

    public function defaultOptionalFields():array{
        $optionalColumns = [
            [
                'name' => 'title',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 5,  // Length of the varchar field
            ],
            [
                'name' => 'name',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 40,
            ],
            [
                'name' => 'address',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 40,
            ],
            [
                'name' => 'city',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 20,
            ],
            [
                'name' => 'state',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 15,
            ],
            [
                'name' => 'zip',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 10,
            ],
            [
                'name' => 'oc',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 40,
            ],
            [
                'name' => 'paidamt',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 10,
            ],
            [
                'name' => 'paiddate',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 10,
            ],
            [
                'name' => 'comments',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 60,
            ],
            [
                'name' => 'countynum',
                'type' => 'string',
                'nullable' => true,
                'default' => null,
                'length' => 5,
            ],
        ];
        return $optionalColumns;
    }
}