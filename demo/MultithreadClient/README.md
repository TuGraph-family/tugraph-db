# multi_thread_client
poc test mutil-thread client

## how to start
use follow command to start
* execute callcypher
```
./multi_thread_client --host 127.0.0.1:9091 -m callcypher -t 10 --input ./callcypher.conf --output ./callcypher.result --continue_on_error 1 -u admin --password 73@TuGraph
```
* execute callplugin
```
./multi_thread_client --host 127.0.0.1:9091 -m callplugin -t 10 --input ./callplugin.conf --output ./callplugin.result --continue_on_error 1 -u admin --password 73@TuGraph
```

* execute loadplugin
```
./multi_thread_client --host 127.0.0.1:9091 -u admin --password 73@TuGraph -m loadplugin -t 1 --continue_on_error true --input ./loadplugin.conf --output ./loadplugin.result
```
* execute deleteplugin
```
./multi_thread_client --host 127.0.0.1:9091 -u admin --password 73@TuGraph -m deleteplugin -t 1 --continue_on_error true --input ./deleteplugin.conf --output ./deleteplugin.result
```
## parameter  introduce
--host                  TuGraph address and port
-m/--mode               start mode(callcypher, callplugin, loadplugin, deleteplugin)
-t/--thread             client thread number, if value Is greater than cpu number，value is cpu number
--input                 the configuration file，contains commands to execute
--output                result file
-u/--user               user name
--password              password
--continue_on_error     ignore when an error occurs