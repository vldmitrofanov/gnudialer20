<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use App\Models\CampaignFormSchema;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Schema;
use Maatwebsite\Excel\Facades\Excel;
use Illuminate\Support\Facades\Storage;
use League\Csv\Writer;

class LeadController extends Controller
{
    public function index(Request $request)
    {
        $request->validate([
            'campaign' => 'required',
        ]);
        $campaign = $request->campaign;
        $order = $request->order ? $request->order : 'desc';
        $table_name = 'campaign_' . $campaign;
        $perPage = $request->perPage ? $request->perPage : 30;
        $orderBy = $request->orderBy ? $request->orderBy : 'id';
        $leads = DB::table($table_name);
        $qFieldName = $request->filterBy;
        $q = $request->filter;
        $qCondition = $request->filterCondition;
        $sqlFilter = !empty($request->sqlFilter) ? $request->sqlFilter : null;
        if (!empty($sqlFilter)) {
            $leads = $leads->whereRaw($sqlFilter);
        } else {
            if (!empty($qFieldName) && !empty($q)) {
                switch ($qCondition) {
                    case "=":
                    case ">=":
                    case "<=":
                    case "<":
                    case ">":
                    case "!=":
                        $leads = $leads->where($qFieldName, $qCondition, $q);
                        break;
                    case "like_%t%":
                        $leads = $leads->where($qFieldName, 'LIKE', "%" . $q . "%");
                        break;
                    case "like_t%":
                        $leads = $leads->where($qFieldName, 'LIKE', $q . "%");
                        break;
                    case "like_%t":
                        $leads = $leads->where($qFieldName, 'LIKE', "%" . $q);
                        break;
                }
            }
        }

        $leads = $leads->orderBy($orderBy, $order);

        if ($request->has('export') && $request->export > 0) {
            return $this->exportLeads($leads, $campaign, $table_name);
        }

        $leads = $leads->paginate($perPage);

        $schema = CampaignFormSchema::where('table_name', $table_name)->first();
        return response()->json([
            'leads' => $leads,
            'schema' => $schema?->schema
        ]);
    }

    public function upload(Request $request)
    {
        $request->validate([
            'file' => 'required|file|mimes:csv,xlsx,xls|max:49152', // Ensure the file is CSV, XLS, or XLSX and max 2MB
            'campaign_name' => 'required|string'
        ]);
        $campaignName = $request->campaign_name;
        $file = $request->file('file');

        $importPath = storage_path('app/imports');
        if (!file_exists($importPath)) {
            mkdir($importPath, 0755, true);
        }

        // Store the file in the 'storage/app/import' directory with its original name
        $file->storeAs('imports', $file->getClientOriginalName());

        // Get field names from the uploaded file (CSV or Excel)
        $uploadedFields = $this->getUploadedFileFields($file);

        // Get field names from the campaign_[name] table
        $tableName = 'campaign_' . $campaignName;
        if (!Schema::hasTable($tableName)) {
            return response()->json(['error' => 'Table not found'], 404);
        }

        $tableFields = Schema::getColumnListing($tableName);

        // Return JSON with both field names
        return response()->json([
            'uploadedFields' => $uploadedFields,
            'tableFields' => $tableFields,
            'fileName' => $file->getClientOriginalName()
        ]);
    }

    public function importUploaded(Request $request)
    {
        $request->validate([
            'mapping' => 'required|array',
            'file_name' => 'required|string',
            'campaign_name' => 'required|string'
        ]);

        $filePath = storage_path('app/imports/' . $request->file_name);
        if (!Storage::exists('imports/' . $request->file_name)) {
            return response()->json(['error' => 'File not found'], 422);
        }

        // Check if the campaign table exists
        $tableName = 'campaign_' . $request->campaign_name;
        if (!Schema::hasTable($tableName)) {
            return response()->json(['error' => 'Table not found'], 422);
        }

        // Initialize variables for stats
        $imported = 0;
        $skipped = 0;
        $batchSize = 100; // Number of rows to insert at a time
        $batchData = [];

        // Open and read the file
        $extension = pathinfo($filePath, PATHINFO_EXTENSION);
        $header = [];
        $rows = [];
        $totalCount = 0;

        $initialCount = DB::table($tableName)->count();

        if ($extension === 'csv') {
            if (($handle = fopen($filePath, 'r')) !== false) {
                $header = fgetcsv($handle); // Get the header (field names)

                // Process file row by row
                while (($row = fgetcsv($handle, 1000, ',')) !== false) {
                    $mappedData = $this->mapRowToTable($row, $header, $request->mapping);

                    if (!empty($mappedData)) {
                        $batchData[] = $mappedData;
                    }

                    // Insert batch when batch size is reached
                    $cnt = count($batchData);
                    if ($cnt === $batchSize) {
                        DB::table($tableName)->insertOrIgnore($batchData); // Insert batch into the table
                        $batchData = []; // Reset batch
                    }
                    $totalCount += $cnt;
                }

                // Insert remaining rows
                if (!empty($batchData)) {
                    DB::table($tableName)->insertOrIgnore($batchData);
                    $totalCount += count($batchData);
                }

                fclose($handle);
            }
        } elseif (in_array($extension, ['xls', 'xlsx'])) {
            // Handle Excel file using Maatwebsite/Excel
            $rows = Excel::toArray([], $filePath)[0];
            $header = array_shift($rows); // First row is the header

            foreach ($rows as $row) {
                $mappedData = $this->mapRowToTable($row, $header, $request->mapping);

                if (!empty($mappedData)) {
                    $batchData[] = $mappedData;
                }

                // Insert batch when batch size is reached
                if (count($batchData) === $batchSize) {
                    DB::table($tableName)->insertOrIgnore($batchData);
                    $batchData = [];
                }
            }

            // Insert remaining rows
            if (!empty($batchData)) {
                DB::table($tableName)->insertOrIgnore($batchData);
            }
        }

        $finalCount = DB::table($tableName)->count();

        $imported = $finalCount - $initialCount;
        $skipped = $totalCount - $imported;
        createTzpopulateFile($tableName);
        // Return import statistics
        return response()->json([
            'message' => 'Sucessfully imported',
            'stats' => [
                'imported' => $imported,
                'skipped' => $skipped,
                'total' => $totalCount
            ]
        ]);
    }

    private function mapRowToTable($row, $header, $mapping)
    {
        $data = [];

        foreach ($mapping as $map) {
            $uploadedField = $map['uploadedField'];
            $tableField = $map['tableField'];
            $fieldType = $map['fieldType'];

            if ($fieldType === 'custom') {
                // Handle custom fields
                $data[$tableField] = $uploadedField;
            } else {
                // Map the CSV/excel row to the table field
                $index = array_search($uploadedField, $header);
                if ($index !== false && isset($row[$index])) {
                    $data[$tableField] = !empty($row[$index]) ? $row[$index] : null;
                }
            }
        }

        return $data;
    }

    private function getUploadedFileFields($file)
    {
        $extension = $file->getClientOriginalExtension();
        $fields = [];

        if ($extension === 'csv') {
            if (($handle = fopen($file->getRealPath(), 'r')) !== false) {
                // Get the first row (field names)
                $fields = fgetcsv($handle);
                fclose($handle);
            }
        } elseif (in_array($extension, ['xls', 'xlsx'])) {
            // Handle Excel file using Laravel Excel (maatwebsite/excel)
            $fields = Excel::toArray([], $file)[0][0]; // Get the first row as field names
        }

        return $fields;
    }

    protected function exportLeads($query, $campaign, $table_name)
    {
        // Prepare CSV file name and storage path
        $fileName = 'campaign_' . $campaign . '_' . now()->timestamp . '.csv';

        if (!Storage::exists('exports')) {
            Storage::makeDirectory('exports');
        }

        // Create or open the CSV file for writing
        $csvPath = storage_path('app/exports/' . $fileName);
        $csv = Writer::createFromPath($csvPath, 'w+');

        // Get headers (column names)
        $headers = Schema::getColumnListing($table_name);
        $csv->insertOne($headers); // Write the headers to the CSV file

        // Chunk the data to process large datasets
        $query->chunk(1000, function ($leads) use ($csv) {
            foreach ($leads as $lead) {
                // Convert the lead object to an array and write it to the CSV
                $csv->insertOne(get_object_vars($lead));
            }
        });

        // Store the CSV in the local storage (or S3 if configured)
        Storage::disk('local')->put('exports/' . $fileName, file_get_contents($csvPath));

        // Return the download link
        return response()->json([
            'message' => 'CSV exported successfully',
            'download_url' => route('download.export', ['fileName' => $fileName]),
        ]);
    }
}
