# 增加Cypher查询

1. deps/build_deps.sh 增加编译参数支持-fPIC
2. lgraph_galaxy.h 增加token，并且将变量变成protected
3. python_api.cpp继承Galaxy，同时增加Cypher语句查询

## TODO
1. python_api.cpp直接暴露继承后的CGalaxy

