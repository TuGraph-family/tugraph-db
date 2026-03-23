#!/bin/bash

set -euo pipefail

pkill -f lgraph_server || true

deadline=$((SECONDS + 10))
while true; do
    if ! pgrep -f lgraph_server > /dev/null; then
        break
    fi

    if (( SECONDS >= deadline )); then
        echo "Timed out waiting for lgraph_server to stop" >&2
        exit 1
    fi

    sleep 0.1
done
