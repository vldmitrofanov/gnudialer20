<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Storage;

class ExportController extends Controller
{
    public function download($fileName)
    {
        $filePath = 'exports/' . $fileName;

        if (Storage::disk('local')->exists($filePath)) {
            return response()->download(storage_path('app/' . $filePath));
        }

        return abort(404);
    }
}
