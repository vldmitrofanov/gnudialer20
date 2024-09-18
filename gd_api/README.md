

## Linking export directory

```bash
php artisan storage:link
```

## TZpopulate script
```bash
#!/bin/bash
# Define the directory where the files are stored
DIR="/path/to/laravel/storage/app/gnudialer/tzpopulate"

# Loop through all the .tzpl files in the directory
for FILE in "$DIR"/*.tzpl; do
    # Check if there are any .tzpl files in the directory
    [ -e "$FILE" ] || continue

    # Extract the table name from the file name (strip the .tzpl extension)
    TABLE_NAME=$(basename "$FILE" .tzpl)

    # Run the gnudialer --tzpopulate command for the table
    gnudialer --tzpopulate "$TABLE_NAME"

    # If the command was successful, delete the file
    if [ $? -eq 0 ]; then
        rm "$FILE"
    else
        echo "Failed to run gnudialer for table $TABLE_NAME" >&2
    fi
done
```

## Token to communicate with API
```php
$user = User::where('email','asterisk1@example.com')->first();
$token = $user->createToken('ServerToken')->plainTextToken;
```

## example of config file
```bash
cat /etc/gnudialer.conf 
[gnudialer]
gnudpass = gnudpass123

[database]
mysql_username = root
mysql_password = 1dva3
mysql_host = 127.0.0.1
mysql_dbname = gnudialer
mysql_port = 3306

[manager]
manager_username = manager
manager_password = mysecret123

[asterisk]
master_host = 127.0.0.1
server_id = 1

[ari]
ari_username = apiuser
ari_password = secret123
ari_proto = http

[redis]
redis_password = 
redis_port = 6379
redis_host = 127.0.0.1

[api]
api_host=http://127.0.0.1:8000
api_user_id=6
api_user_secret=17|Pvw6kR1tUbZdOowKcEboPxL5Vd22zvWUMMyJIQ3Y444a8f70
```
