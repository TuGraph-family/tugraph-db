#!/bin/bash

./build/clientdemo -i 127.0.0.1 -p 9090 -u admin --password 73@TuGraph -g default -c "MATCH (n) RETURN n LIMIT 100"