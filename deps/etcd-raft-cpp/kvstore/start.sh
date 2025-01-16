#! /bin/bash
set -e

if [ $# -gt 1 ]; then
    echo "Usage: $0 [n1|n2|n3]"
    exit 1
fi

script_path=$(realpath "$0")
script_dir=$(dirname "$script_path")
cd $script_dir

function start_n1() {
  mkdir -p n1 && cp -f ../build/kvstore n1/ && cd n1
  ./kvstore --node_id 1 --raft_port 12379 --http_port 12380 &
}

function start_n2() {
  mkdir -p n2 && cp -f ../build/kvstore n2/ && cd n2
  ./kvstore --node_id 2 --raft_port 22379 --http_port 22380 &
}

function start_n3() {
  mkdir -p n3 && cp -f ../build/kvstore n3/ && cd n3
  ./kvstore --node_id 3 --raft_port 32379 --http_port 32380 &
}

function start_all() {
  mkdir -p n1 n2 n3
  cp -f ../build/kvstore n1/
  cp -f ../build/kvstore n2/
  cp -f ../build/kvstore n3/

  cd n1
  ./kvstore --node_id 1 --raft_port 12379 --http_port 12380 &
  sleep 3

  cd ../n2
  ./kvstore --node_id 2 --raft_port 22379 --http_port 22380 &

  cd ../n3
  ./kvstore --node_id 3 --raft_port 32379 --http_port 32380 &
}

case "$1" in
    n1)
        start_n1
        ;;
    n2)
        start_n2
        ;;
    n3)
        start_n3
        ;;
    "")
        start_all
        ;;
    *)
        echo "Invalid parameter. Expected one of: n1, n2, n3"
        exit 1
        ;;
esac