<?php

use App\Http\Resources\UserAgentResource;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\Route;
use Illuminate\Support\Facades\Auth;


Route::post('/login', function (Request $request) {
    $credentials = $request->only('email', 'password');

    if (!Auth::attempt($credentials)) {
        return response()->json(['message' => 'Invalid login credentials'], 401);
    }

    $user = Auth::user();
    $token = $user->createToken('auth_token')->plainTextToken;

    return response()->json([
        'token' => $token,
        'user' => $user,
    ]);
});

Route::group(['middleware'=>['auth:sanctum']], function () {
    Route::get('/me', function (Request $request) {
        $user = $request->user();
        $user->load('agents');
        return new UserAgentResource($user);
    });

    Route::group(['prefix' => 'server'], function() {
        Route::get('/',[\App\Http\Controllers\ServerController::class, 'index']);
        Route::get('/{server_id}',[\App\Http\Controllers\ServerController::class, 'show']);
    });
});
