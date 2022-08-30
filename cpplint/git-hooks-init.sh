#!/bin/bash
set -e

DIR=$(dirname "$(readlink -f "$0")")
cd $DIR/../.git/hooks
rm -f pre-push
ln -s ../../cpplint/pre-push pre-push
