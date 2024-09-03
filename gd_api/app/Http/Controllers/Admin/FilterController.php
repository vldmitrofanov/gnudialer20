<?php

namespace App\Http\Controllers\Admin;

use App\Http\Controllers\Controller;
use Illuminate\Http\Request;

class FilterController extends Controller
{
    public function create(Request $request)
    {
        $request->validate([
            'queue_id' => 'required',
            'filter' => 'required',
        ]);
        if (!$this->areParenthesesBalanced($request->filter)) {
            return response()->json(['success' => false, 'message' => 'Please check your SQL string'],422);
        }
        $filter = \App\Models\Filter::create([
            'enabled' => !empty($request->enabled) ? 1 : 0,
            'position' => empty($request->position) ? \App\Models\Filter::where('queue_id', $request->queue_id)->count() + 5 : $request->position,
            'queue_id' => $request->queue_id,
            'filter' => $request->filter
        ]);      
        if(!empty($filter)) {
            return response()->json(['success' => true],200);
        }
        return response()->json(['success' => false],422);
    }

    public function update($filter_id, Request $request)
    {
        $request->validate([
            'queue_id' => 'required',
            'filter' => 'required',
        ]);
        if (!$this->areParenthesesBalanced($request->filter)) {
            return response()->json(['success' => false, 'message' => 'Please check your SQL string'],422);
        }
        $filter = \App\Models\Filter::findOrFail($filter_id);
        $filter->update($request->all());
        return response()->json(['success' => true],200);
    }

    public function delete($filter_id) {
        \App\Models\Filter::findOrFail($filter_id)->delete();
        return response()->json(['success' => true],200);
    }

    private function areParenthesesBalanced($string) {
        // Count the number of opening parentheses '('
        $openCount = substr_count($string, '(');
        
        // Count the number of closing parentheses ')'
        $closeCount = substr_count($string, ')');
        
        // Check if both counts are equal
        return $openCount === $closeCount;
    }
}
