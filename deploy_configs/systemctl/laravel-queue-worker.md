/etc/systemd/system/laravel-queue-worker.service
```ini
[Unit]
Description=Laravel Queue Worker
After=network.target

[Service]
# The user that should run the worker (usually "www-data" for Apache/Nginx)
User=www-data
# The group that should run the worker
Group=www-data
# The directory containing your Laravel application
WorkingDirectory=/home/debian/gnudialer20/gd_api
# Command to run the Laravel queue worker
ExecStart=/usr/bin/php /home/debian/gnudialer20/gd_api/artisan queue:work --tries=3 --timeout=60
# Restart automatically on failure
Restart=on-failure
# Timeout to avoid hanging processes
RestartSec=5
# Ensure the system environment is loaded correctly
Environment=APP_ENV=production
Environment=QUEUE_CONNECTION=database

[Install]
WantedBy=multi-user.target
```