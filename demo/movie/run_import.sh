#!/bin/bash
mkdir -p /var/lib/lgraph/
lgraph_import --dir /var/lib/lgraph/movie_db --verbose 2 -c import.json --dry_run 0 --continue_on_error 1 --overwrite 1 --online false
rm -rf import_tmp

echo "IMPORT DONE."

