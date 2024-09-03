<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;

class SettingsController extends Controller
{
    public function create(Request $request) {
        $data = $request->validate([
            'queue_id' => 'required',
            'parameter' => 'required|string',
            'value' => 'required',
            'type' => 'nullable|string'
        ]);
        $s = \App\Models\Setting::create($data);
        return response()->json(['success' => true, 'data' => $s],200);
    }

    public function update($setting_id, Request $request) {
        $request->validate([
            'value' => 'required'
        ]);
        $s = \App\Models\Setting::findOrFail($setting_id);
        $s->update([
            'value' => $request->value
        ]);
        return response()->json(['success' => true, 'data' => $s],200);
    }

    public function delete($setting_id) {
        \App\Models\Setting::findOrFail($setting_id)->delete();
        return response()->json(['success' => true],200);
    }
}
