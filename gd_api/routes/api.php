<?php

use Illuminate\Support\Facades\Route;


Route::post('/login', [\App\Http\Controllers\UserController::class, 'login']);

Route::group(['middleware' => ['auth:sanctum']], function () {
    Route::get('/me', [\App\Http\Controllers\UserController::class, 'me']);

    Route::group(['prefix' => 'server'], function () {
        Route::get('/', [\App\Http\Controllers\ServerController::class, 'index']);
        Route::get('/{server_id}', [\App\Http\Controllers\ServerController::class, 'show']);
    });

    Route::group(['prefix' => 'queue'], function () {
        Route::get('/', [\App\Http\Controllers\QueueController::class, 'agentIndex']);
    });

    Route::group(['prefix' => 'asterisk'], function () {
        Route::post('/agent/pause', [\App\Http\Controllers\AsteriskController::class, 'setAgentPause']);
        Route::get('/agent/status', [\App\Http\Controllers\AsteriskController::class, 'getAgentStatus']);
        Route::post('/call/hangup', [\App\Http\Controllers\AsteriskController::class, 'callHangup']);
        Route::post('/custom/user-action', [\App\Http\Controllers\AsteriskController::class, 'userAction']);
    });

    Route::group(['prefix' => 'leads'], function () {
        Route::get('/', [\App\Http\Controllers\LeadController::class, 'getLead']);
        Route::get('/search', [\App\Http\Controllers\LeadController::class, 'searchLead']);
    });
});
