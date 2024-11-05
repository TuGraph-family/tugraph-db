#!/bin/bash

pkill -f lgraph_server

while true; do
    if ! pgrep -f lgraph_server > /dev/null; then
        break
    fi
    sleep 0.1
done

SCRIPT_DIR="$( cd -- "$( dirname -- $0 )" >/dev/null 2>&1 && pwd )"
cd $SCRIPT_DIR/../../../../build/
rm -rf data && mkdir data

./lgraph_server --mode=run --data_path=data --log_path=log --log_level=debug > stdout.log 2>&1 &
while true; do
    if ss -tuln | grep -q ":7687"; then
        break
    fi
    sleep 0.1
done
