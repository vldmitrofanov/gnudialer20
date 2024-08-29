<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

class CustomCorsMiddleware
{
    public function handle(Request $request, $next) // Remove the type hint for Closure
    {
        $response = $next($request);

        // Add CORS headers
        $response->headers->set('Access-Control-Allow-Origin', '*');
        $response->headers->set('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
        $response->headers->set('Access-Control-Allow-Headers', 'Content-Type, Authorization');
        $response->headers->set('Access-Control-Allow-Credentials', 'true');

        // If the request method is OPTIONS, return an empty response with the headers
        if ($request->getMethod() === 'OPTIONS') {
            return response()->json('', 200, $response->headers->all());
        }

        return $response;
    }
}
