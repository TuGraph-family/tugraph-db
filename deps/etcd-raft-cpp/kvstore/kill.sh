#! /bin/bash
set -e

if [ $# -gt 1 ]; then
    echo "Usage: $0 [n1|n2|n3]"
    exit 1
fi

case "$1" in
    n1)
        ps -ef | grep kvstore | grep "node_id 1" | awk '{print $2}' | xargs kill -9
        ;;
    n2)
        ps -ef | grep kvstore | grep "node_id 2" | awk '{print $2}' | xargs kill -9
        ;;
    n3)
        ps -ef | grep kvstore | grep "node_id 3" | awk '{print $2}' | xargs kill -9
        ;;
    "")
        pgrep -f kvstore | xargs kill -9
        ;;
    *)
        echo "Invalid parameter. Expected one of: n1, n2, n3"
        exit 1
        ;;
esac

