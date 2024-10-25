<?php

use Illuminate\Support\Facades\Route;
use Illuminate\Support\Facades\Storage;

Route::post('/login', [\App\Http\Controllers\UserController::class, 'login']);
Route::post('/logout', [\App\Http\Controllers\UserController::class, 'logout']);

Route::group(['middleware' => ['auth:sanctum']], function () {
    Route::get('/me', [\App\Http\Controllers\UserController::class, 'me']);

    Route::group(['middleware' => ['auth.admin']], function () {
        Route::group(['prefix' => 'admin'], function () {
            Route::group(['prefix' => 'servers'], function () {
                Route::get('/', [\App\Http\Controllers\Admin\ServerController::class, 'index']);
            });

            Route::group(['prefix' => 'campaigns'], function () {
                Route::get('/', [\App\Http\Controllers\Admin\CampaignController::class, 'index']);
                Route::get('/{campaign_id}', [\App\Http\Controllers\Admin\CampaignController::class, 'show']);
                Route::post('/', [\App\Http\Controllers\Admin\CampaignController::class, 'create']);
                Route::delete('/{campaign_id}', [\App\Http\Controllers\Admin\CampaignController::class, 'delete']);
            });

            Route::group(['prefix' => 'filters'], function () {
                Route::post('/', [\App\Http\Controllers\Admin\FilterController::class, 'create']);
                Route::put('/{filter_id}', [\App\Http\Controllers\Admin\FilterController::class, 'update']);
                Route::delete('/{filter_id}', [\App\Http\Controllers\Admin\FilterController::class, 'delete']);
            });

            Route::group(['prefix' => 'settings'], function () {
                Route::post('/', [\App\Http\Controllers\Admin\SettingsController::class, 'create']);
                Route::put('/{filter_id}', [\App\Http\Controllers\Admin\SettingsController::class, 'update']);
                Route::delete('/{filter_id}', [\App\Http\Controllers\Admin\SettingsController::class, 'delete']);
            });

            Route::group(['prefix' => 'agents'], function () {
                Route::get('/active-conf-bridges', [\App\Http\Controllers\Admin\AgentController::class, 'getActiveConfBridges']);
                Route::get('/', [\App\Http\Controllers\Admin\AgentController::class, 'index']);
                Route::post('/', [\App\Http\Controllers\Admin\AgentController::class, 'create']);
                Route::put('/{agent_id}', [\App\Http\Controllers\Admin\AgentController::class, 'update']);
                Route::delete('/{agent_id}', [\App\Http\Controllers\Admin\AgentController::class, 'delete']);              
            });

            Route::group(['prefix' => 'queues'], function () {
                Route::post('/', [\App\Http\Controllers\Admin\QueueController::class, 'create']);
                Route::put('/{queue_id}/agents', [\App\Http\Controllers\Admin\QueueController::class, 'updateAgents']);
                Route::delete('/{queue_id}', [\App\Http\Controllers\Admin\QueueController::class, 'delete']);
            });

            Route::group(['prefix' => 'leads'], function () {
                Route::get('/', [\App\Http\Controllers\Admin\LeadController::class, 'index']);
                Route::post('/upload', [\App\Http\Controllers\Admin\LeadController::class, 'upload']);
                Route::post('/import-uploaded', [\App\Http\Controllers\Admin\LeadController::class, 'importUploaded']);
            });

            Route::group(['prefix' => 'dispositions'], function () {
                Route::get('/', [\App\Http\Controllers\Admin\DispositionController::class, 'index']);
            });

            Route::get('/export/{fileName}', [\App\Http\Controllers\Admin\ExportController::class, 'download'])->name('download.export');
        });
    });

    Route::group(['prefix' => 'server'], function () {
        Route::get('/', [\App\Http\Controllers\ServerController::class, 'index']);
        Route::get('/{server_id}', [\App\Http\Controllers\ServerController::class, 'show']);
    });

    Route::group(['prefix' => 'queue'], function () {
        Route::get('/', [\App\Http\Controllers\QueueController::class, 'agentIndex']);
        Route::get('/{queue_id}/get-next-lead', [\App\Http\Controllers\QueueController::class, 'getNextLead']);
    });

    Route::group(['prefix' => 'asterisk'], function () {
        Route::post('/agent/pause', [\App\Http\Controllers\AsteriskController::class, 'setAgentPause']);
        Route::post('/agent/available', [\App\Http\Controllers\AsteriskController::class, 'setAgentAvailable']);
        Route::get('/agent/status', [\App\Http\Controllers\AsteriskController::class, 'getAgentStatus']);
        Route::post('/call-ari', [\App\Http\Controllers\AsteriskController::class,'callLeadByID_ARI']);
        Route::post('/call-ami', [\App\Http\Controllers\AsteriskController::class,'callLeadByID_AMI']);
        Route::post('/hold-ami', [\App\Http\Controllers\AsteriskController::class,'toggleHold']);
        Route::post('/call/hangup', [\App\Http\Controllers\AsteriskController::class, 'callHangup']);
        Route::post('/custom/user-action', [\App\Http\Controllers\AsteriskController::class, 'userAction']);
        Route::post('/bridge-3way' , [\App\Http\Controllers\AsteriskController::class, 'bridge3way']);
        Route::post('/leave-3way' , [\App\Http\Controllers\AsteriskController::class, 'leave3way']);
    });

    Route::group(['prefix' => 'leads'], function () {
        Route::get('/', [\App\Http\Controllers\LeadController::class, 'getLead']);
        Route::put('/', [\App\Http\Controllers\LeadController::class, 'updateLead']);
        Route::get('/search', [\App\Http\Controllers\LeadController::class, 'searchLead']);
    });

    Route::group(['prefix' => 'dispositions'], function () {
        Route::post('/', [\App\Http\Controllers\DispositionController::class, 'create']);
    });
    
});
