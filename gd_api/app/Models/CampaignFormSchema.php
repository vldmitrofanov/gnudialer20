<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use Illuminate\Support\Facades\Schema;
use Illuminate\Support\Str;

class CampaignFormSchema extends Model
{
    use HasFactory;
    protected $fillable = [
        'table_name',
        'schema'
    ];

    protected $casts = [
        'schema' => 'array'
    ];

    public $hiddenFields = [
        'tzl',
        'tzh',
        'attempts',
        'pickups',
        'abandons',
        'disposition',
        'agent',
        'closerdispo',
        'closer',
        'subdispo',
        'lastupdated',
        'cb_datetime',
        'wdayl',
        'wdayh',
        'satl',
        'sath',
        'sunl',
        'sunh',
        'holl',
        'holh'
    ];

    private $readOnly = [
        'id',
        'phone'
    ];

    private $textareaType = [
        'comments'
    ];

    function getTableColumns($tableName):array
    {
        // Get an array of column names for the specified table
        $columns = Schema::getColumnListing($tableName);

        $filteredColumns = array_diff($columns, $this->hiddenFields);

        return $filteredColumns;
    }

    function makeSchema(array $columns):array
    {
        $results = [];
        foreach ($columns as $column){
            $results[] = [ 
                            'name' => $column,
                            'label'=> Str::title(str_replace('_', ' ', $column)),
                            'field_type' => in_array($column,$this->textareaType)?'textarea':'input',
                            'read_only' => in_array($column,$this->readOnly)?1:0
                        ];
        }
        return $results;
    }
}
