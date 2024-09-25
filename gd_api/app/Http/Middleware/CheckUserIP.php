<?php

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;
use Illuminate\Support\Facades\Cache;
use Illuminate\Support\Facades\Auth;

class CheckUserIP
{
    /**
     * Handle an incoming request.
     *
     * @param  \Closure(\Illuminate\Http\Request): (\Symfony\Component\HttpFoundation\Response)  $next
     */
    public function handle(Request $request, Closure $next): Response
    {
        if (Auth::check()) {
            $user = Auth::user();
            $currentIp = $request->ip();

            // Retrieve the cached IP address for the user
            $cachedIp = Cache::get('user_ip_' . $user->id);

            // If the cached IP is not set or doesn't match the current IP
            if (!$cachedIp || $cachedIp !== $currentIp) {
                // Update the IP address in the database
                $user->ip_address = $currentIp;
                $user->save();

                // Cache the new IP address (set the cache to last for a day)
                Cache::put('user_ip_' . $user->id, $currentIp, 86400); // 86400 seconds = 1 day
            }
        }

        return $next($request);
    }
}
