

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